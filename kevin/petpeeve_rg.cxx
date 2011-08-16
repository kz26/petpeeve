// Automated tumor detection in PET images
// Kevin Zhang
// Bria Connolly
// Baseline RG method

#include "itktypes.h"
#include "ext_functions.h"

int main(int argc, char* argv[])
{

    if (argc < 7)
    {
	    std::cerr << "Usage: " <<  argv[0] << " input_dcm_directory output_dir raw1_output_dir raw2_output_dir sigma convex_height [# threads]" << std::endl;
        return -1;
    }	

    int sigma = atoi(argv[5]);
    int convex_height = atoi(argv[6]);
    int num_threads;
    if (argc == 8)
        num_threads = atoi(argv[7]);
    else
        num_threads = 1;

    //std::cerr << "Number of args: " << argc << std::endl;
    std::cerr << "Baseline method" << std::endl;
    std::cerr << "Number of threads: " << num_threads << std::endl;
    std::cerr << "Sigma value: " << sigma << std::endl;
    std::cerr << "Convex height: " << convex_height << std::endl;

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

    InvertIntensityFilterType::Pointer InvertFilter = InvertIntensityFilterType::New();
    InvertFilter->SetInput(OtsuFilter->GetOutput());

    // Initialize structuring element (binary)
    BBStructuringElementBinType BBStructuringElementBin;
    BBStructuringElementBin.CreateStructuringElement();

    // begin binary dilation filter
    BBStructuringElementBin.SetRadius(5);
    DilateFilterType::Pointer BinaryDilateFilter = DilateFilterType::New();
    BinaryDilateFilter->SetKernel(BBStructuringElementBin);
    BinaryDilateFilter->SetDilateValue(255);
    BinaryDilateFilter->SetInput(InvertFilter->GetOutput());

    // begin binary erosion filter
    BBStructuringElementBin.SetRadius(2);
    ErodeFilterType::Pointer BinaryErodeFilter = ErodeFilterType::New();
    BinaryErodeFilter->SetKernel(BBStructuringElementBin);
    BinaryErodeFilter->SetErodeValue(255);
    BinaryErodeFilter->SetInput(BinaryDilateFilter->GetOutput());

    CCFilterType::Pointer MaskCCFilter = CCFilterType::New();
    MaskCCFilter->SetNumberOfThreads(num_threads);
    MaskCCFilter->SetInput(BinaryErodeFilter->GetOutput());

    RelabelFilterType::Pointer MaskRelabelFilter = RelabelFilterType::New();
    MaskRelabelFilter->SetNumberOfThreads(num_threads);
    MaskRelabelFilter->SetInput(MaskCCFilter->GetOutput());

    MaskBTFilterType::Pointer MaskBTFilter = MaskBTFilterType::New();
    MaskBTFilter->SetOutsideValue(0);
    MaskBTFilter->SetInsideValue(255);
    MaskBTFilter->SetLowerThreshold(1);
    MaskBTFilter->SetUpperThreshold(1);
    MaskBTFilter->SetInput(MaskRelabelFilter->GetOutput());

    // Apply mask
    MaskFilterType::Pointer MaskFilter = MaskFilterType::New();
    MaskFilter->SetInput1(DCMToFloatFilter->GetOutput());
    MaskFilter->SetInput2(MaskBTFilter->GetOutput());

    // END BODY SEGMENTATION
    // BEGIN OBJECT DETECTION
    
    // Apply recursive Gaussian blur
    RGFilterType::Pointer RGFilter = RGFilterType::New();
    RGFilter->SetNormalizeAcrossScale(false);
    RGFilter->SetSigma(sigma);
    RGFilter->SetNumberOfThreads(num_threads);
    RGFilter->SetInput(MaskFilter->GetOutput());

    ConvexFilterType::Pointer ConvexFilter = ConvexFilterType::New();
    ConvexFilter->SetInput(RGFilter->GetOutput());
    ConvexFilter->SetNumberOfThreads(num_threads);
    ConvexFilter->SetHeight(convex_height);

    FloatRescaleIntensityFilterType::Pointer FloatRescaleFilter = FloatRescaleIntensityFilterType::New();
    FloatRescaleFilter->SetOutputMinimum(0);
    FloatRescaleFilter->SetOutputMaximum(255);
    FloatRescaleFilter->SetInput(ConvexFilter->GetOutput());
    FloatRescaleFilter->Update();

    ThresholdFilterType::Pointer ThresholdFilter = ThresholdFilterType::New();
    ThresholdFilter->SetOutsideValue(0);
    ThresholdFilter->ThresholdBelow(128);
    ThresholdFilter->SetInput(FloatRescaleFilter->GetOutput());
    ThresholdFilter->SetNumberOfThreads(num_threads);

    BinaryThresholdFilterType::Pointer BinaryThresholdFilterC = BinaryThresholdFilterType::New();
    BinaryThresholdFilterC->SetInsideValue(255);
    BinaryThresholdFilterC->SetOutsideValue(0);
    BinaryThresholdFilterC->SetLowerThreshold(1); 
    BinaryThresholdFilterC->SetUpperThreshold(255);
    BinaryThresholdFilterC->SetInput(FloatRescaleFilter->GetOutput());

    CCFilterType::Pointer CCFilter = CCFilterType::New();
    CCFilter->SetNumberOfThreads(num_threads);
    CCFilter->SetInput(BinaryThresholdFilterC->GetOutput());
    CCFilter->SetMaskImage(BinaryThresholdFilterC->GetOutput());
    
    unsigned int min_object_size = 2;
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

    // Convert back to DCM pixel type
    FloatToDCMFilterType::Pointer FloatToDCMFilter = FloatToDCMFilterType::New();
    FloatToDCMFilter->SetInput(RGFilter->GetOutput());

    //FloatToDCMFilterType::Pointer FloatToDCMFilter2 = FloatToDCMFilterType::New();
    //FloatToDCMFilter2->SetInput(RescaleIntensityFilter->GetOutput());

    // Convert from eight-bit to DCM pixel type
    EightBitToDCMFilterType::Pointer EightBitToDCMFilter = EightBitToDCMFilterType::New();
    EightBitToDCMFilter->SetInput(MaskBTFilter->GetOutput());

    // Write end result of pipeline
    // Set up FileSeriesWriter
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(RescaleIntensityFilter->GetOutput()); // Set input
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
    std::cerr << "Labeled output files successfully written." << std::endl;

    // Write raw 1 output files
    WriterType::Pointer RawWriter1 = WriterType::New();
    RawWriter1->SetInput(FloatToDCMFilter->GetOutput()); // set input
    RawWriter1->SetImageIO(dcmIO);
    const char * Raw1OutputDirectory = argv[3];
    itksys::SystemTools::MakeDirectory(Raw1OutputDirectory);
    nameGenerator->SetOutputDirectory(Raw1OutputDirectory);
    RawWriter1->SetFileNames(nameGenerator->GetOutputFileNames());
    RawWriter1->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());

    try
    {
        RawWriter1->Update();
    }
    catch (itk::ExceptionObject & e)
    {
        std::cerr << "Exception caught in RawWriter1." << std::endl;
        std::cerr << e << std::endl;
        return -1;
    }
    std::cerr << "Raw 1 output files successfully written." << std::endl;

    // Write raw 2 output files
    WriterType::Pointer RawWriter2 = WriterType::New();
    RawWriter2->SetInput(EightBitToDCMFilter->GetOutput()); // Set input
    RawWriter2->SetImageIO(dcmIO);
    const char * Raw2OutputDirectory = argv[4];
    itksys::SystemTools::MakeDirectory(Raw2OutputDirectory);
    nameGenerator->SetOutputDirectory(Raw2OutputDirectory);
    RawWriter2->SetFileNames(nameGenerator->GetOutputFileNames());
    RawWriter2->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());

    try
    {
        RawWriter2->Update();
    }
    catch (itk::ExceptionObject & e)
    {
        std::cerr << "Exception caught in RawWriter2." << std::endl;
        std::cerr << e << std::endl;
        return -1;
    }
    std::cerr << "Raw 2 output files successfully written." << std::endl;
 

    return 0;

}
