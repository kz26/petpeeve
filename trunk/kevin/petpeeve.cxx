// Automated tumor detection in PET images
// Kevin Zhang
// Bria Connolly

#include "itktypes.h"
#include "ext_functions.h"

int main(int argc, char* argv[])
{

    if (argc < 5)
    {
	    std::cerr << "Usage: " <<  argv[0] << " input_dcm_directory output_dir raw_output_dir sigma [# threads]" << std::endl;
        return -1;
    }	

    int sigma = atoi(argv[4]);
    int num_threads;
    if (argc == 6)
        num_threads = atoi(argv[5]);
    else
        num_threads = 1;
    //std::cerr << "Number of args: " << argc << std::endl;
    std::cerr << "Number of threads: " << num_threads << std::endl;
    std::cerr << "Sigma value: " << sigma << std::endl;

    ReaderType::Pointer reader = ReaderType::New();

    ImageIOType::Pointer dcmIO = ImageIOType::New();
    reader->SetImageIO(dcmIO);

    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    //nameGenerator->SetUseSeriesDetails(true);
    nameGenerator->SetDirectory(argv[1]);

    FileNamesContainer fileNames;
    fileNames = nameGenerator->GetInputFileNames();
    reader->SetFileNames(fileNames);
    try 
    {
        reader->Update();
    }
    catch(itk::ExceptionObject & e)
    {  
        std::cerr << "Exception caught during file reading." << std::endl;
        std::cerr << e << std::endl;
        return -1;
    }   

    // Cast images to float pixel type
    DCMToFloatFilterType::Pointer DCMToFloatFilter = DCMToFloatFilterType::New();
    DCMToFloatFilter->SetInput(reader->GetOutput());

    // begin Otsu filter
    OtsuFilterType::Pointer OtsuFilter = OtsuFilterType::New();
    OtsuFilter->SetInput(DCMToFloatFilter->GetOutput());

    // Initialize structuring element (binary)
    BBStructuringElementBinType BBStructuringElementBin;
    BBStructuringElementBin.SetRadius(10); 
    BBStructuringElementBin.CreateStructuringElement();

    // begin binary erosion filter
    ErodeFilterType::Pointer BinaryErodeFilter = ErodeFilterType::New();
    BinaryErodeFilter->SetKernel(BBStructuringElementBin);
    BinaryErodeFilter->SetErodeValue(255);
    BinaryErodeFilter->SetInput(OtsuFilter->GetOutput());

    // begin binary dilation filter
    BBStructuringElementBin.SetRadius(1);
    DilateFilterType::Pointer BinaryDilateFilter = DilateFilterType::New();
    BinaryDilateFilter->SetKernel(BBStructuringElementBin);
    BinaryDilateFilter->SetDilateValue(255);
    BinaryDilateFilter->SetInput(BinaryErodeFilter->GetOutput());

    // Apply mask
    MaskFilterType::Pointer MaskFilter = MaskFilterType::New();
    MaskFilter->SetInput1(DCMToFloatFilter->GetOutput());
    //MaskFilter->SetInput2(BinaryDilateFilter->GetOutput());
    MaskFilter->SetInput2(BinaryDilateFilter->GetOutput());

    /*
    // Apply recursive Gaussian blur
    RGFilterType::Pointer RGFilter = RGFilterType::New();
    RGFilter->SetNormalizeAcrossScale(false);
    RGFilter->SetSigma(sigma);
    RGFilter->SetNumberOfThreads(num_threads);
    RGFilter->SetInput(MaskFilter->GetOutput());
    */

    // Apply LoG
    LoGFilterType::Pointer LoGFilter = LoGFilterType::New();
    LoGFilter->SetSigma(sigma);
    LoGFilter->SetInput(MaskFilter->GetOutput());

    /*
    // Rescale image intensity
    FloatRescaleIntensityFilterType::Pointer LoGRescaleIntensityFilter = FloatRescaleIntensityFilterType::New();
    LoGRescaleIntensityFilter->SetOutputMinimum(-1000);
    LoGRescaleIntensityFilter->SetOutputMaximum(1000);
    LoGRescaleIntensityFilter->SetNumberOfThreads(num_threads);
    LoGRescaleIntensityFilter->SetInput(LoGFilter->GetOutput());
    */

    /*
    // Apply 50% thresholding
    ThresholdFilterType::Pointer Threshold50Filter = ThresholdFilterType::New();
    Threshold50Filter->SetOutsideValue(0);
    Threshold50Filter->ThresholdBelow(7000);
    Threshold50Filter->SetNumberOfThreads(num_threads);
    Threshold50Filter->SetInput(LoGFilter->GetOutput());
    */

    // Apply thresholding and convert to binary
    BinaryThresholdFilterType::Pointer BinaryThresholdFilter = BinaryThresholdFilterType::New();
    BinaryThresholdFilter->SetInsideValue(255);
    BinaryThresholdFilter->SetOutsideValue(0);
    BinaryThresholdFilter->SetLowerThreshold(-65536);
    BinaryThresholdFilter->SetUpperThreshold(-65);
    BinaryThresholdFilter->SetInput(LoGFilter->GetOutput());

    // Secondary binary dilation step
    BBStructuringElementBin.SetRadius(1);
    DilateFilterType::Pointer BinaryDilateFilter2 = DilateFilterType::New();
    BinaryDilateFilter2->SetKernel(BBStructuringElementBin);
    BinaryDilateFilter2->SetDilateValue(255);
    BinaryDilateFilter2->SetInput(BinaryThresholdFilter->GetOutput());

    //MaskFilter->Update();
    //DCMImageType::Pointer MultiImage = makeSRGPyramidImage(MaskFilter->GetOutput(), level, num_threads);
    
    /*
    // Apply convex image filter
    ConvexFilterType::Pointer ConvexFilter = ConvexFilterType::New();
    ConvexFilter->SetHeight(100);
    ConvexFilter->SetNumberOfThreads(num_threads);
    //ConvexFilter->FullyConnectedOn();
    //ConvexFilter->SetInput(RGFilter->GetOutput());
    ConvexFilter->SetInput(MultiImage);
    */

    CCFilterType::Pointer CCFilter = CCFilterType::New();
    CCFilter->SetNumberOfThreads(num_threads);
    CCFilter->SetInput(BinaryDilateFilter2->GetOutput());
    CCFilter->SetMaskImage(BinaryDilateFilter2->GetOutput());
    //CCFilter->FullyConnectedOn();
    
    unsigned int min_object_size = 10;
    // Relabel component filter
    RelabelFilterType::Pointer RelabelFilter = RelabelFilterType::New();
    RelabelFilter->SetInput(CCFilter->GetOutput());
    RelabelFilter->SetNumberOfThreads(num_threads);
    RelabelFilter->SetMinimumObjectSize(min_object_size);

    RelabelFilter->Update();
    printCentroids(RelabelFilter);
    unsigned int numobjects = RelabelFilter->GetOriginalNumberOfObjects();
    std::cerr << "Minimum size threshold: " << min_object_size << std::endl;
    std::cerr << "Number of objects detected (all): " << numobjects << std::endl;
    std::cerr << "Number of objects detected (within min size threshold): " << RelabelFilter->GetNumberOfObjects() << std::endl;
    
    // Rescale image intensity
    RescaleIntensityFilterType::Pointer RescaleIntensityFilter = RescaleIntensityFilterType::New();
    RescaleIntensityFilter->SetOutputMinimum(0);
    RescaleIntensityFilter->SetOutputMaximum(1000);
    RescaleIntensityFilter->SetNumberOfThreads(num_threads);
    RescaleIntensityFilter->SetInput(RelabelFilter->GetOutput());

    // Convert from long to DCM pixel type
    //LongToDCMFilterType::Pointer LongToDCMFilter = LongToDCMFilterType::New();
    //LongToDCMFilter->SetInput(CCFilter->GetOutput());

    // Convert back to DCM pixel type
    FloatToDCMFilterType::Pointer FloatToDCMFilter = FloatToDCMFilterType::New();
    FloatToDCMFilter->SetInput(LoGFilter->GetOutput());

    /*
    // Rescale LoGFilter for output purposes
    RescaleIntensityFilterType::Pointer RescaleIntensityFilter2 = RescaleIntensityFilterType::New();
    RescaleIntensityFilter2->SetOutputMinimum(-1000);
    RescaleIntensityFilter2->SetOutputMaximum(1000);
    RescaleIntensityFilter2->SetNumberOfThreads(num_threads);
    RescaleIntensityFilter2->SetInput(FloatToDCMFilter->GetOutput());
    */

    // Convert from eight-bit to DCM pixel type
    EightBitToDCMFilterType::Pointer EightBitToDCMFilter = EightBitToDCMFilterType::New();
    EightBitToDCMFilter->SetInput(BinaryDilateFilter->GetOutput());

    // Write end result of pipeline
    // Set up FileSeriesWriter
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(RescaleIntensityFilter->GetOutput());
    //
    writer->SetImageIO(dcmIO);
    const char * outputDirectory = argv[2];
    itksys::SystemTools::MakeDirectory(outputDirectory);
    nameGenerator->SetOutputDirectory(outputDirectory);
    writer->SetFileNames(nameGenerator->GetOutputFileNames());
    writer->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());

    try
    {
	    writer->Update();
    } 
    catch (itk::ExceptionObject & e)
    { 
	    std::cerr << "Exception caught in writer." << std::endl;
	    std::cerr << e << std::endl;
	    return -1;
    }
    std::cerr << "Output files successfully written." << std::endl;

    // Write raw output files
    // Set up FileSeriesWriter for masks
    RawWriterType::Pointer RawWriter = RawWriterType::New();
    RawWriter->SetInput(EightBitToDCMFilter->GetOutput());
    RawWriter->SetImageIO(dcmIO);
    const char * RawOutputDirectory = argv[3];
    itksys::SystemTools::MakeDirectory(RawOutputDirectory);
    nameGenerator->SetOutputDirectory(RawOutputDirectory);
    RawWriter->SetFileNames(nameGenerator->GetOutputFileNames());
    RawWriter->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());

    try
    {
        RawWriter->Update();
    }
    catch (itk::ExceptionObject & e)
    {
        std::cerr << "Exception caught in RawWriter." << std::endl;
        std::cerr << e << std::endl;
        return -1;
    }
    std::cerr << "Relabeled files successfully written." << std::endl;

    return 0;

}
