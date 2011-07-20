// Kevin Zhang
// Bria Connolly
// Last update: 07/19/2011

#include "itktypes.h"
#include "ext_functions.h"

int main(int argc, char* argv[])
{

    if (argc < 4)
    {
	    std::cerr << "Usage: " <<  argv[0] << " input_dcm_directory output_dir num_levels [# threads]" << std::endl;
        return -1;
    }	

    int level = atoi(argv[3]);
    int num_threads;
    if (argc == 5)
        num_threads = atoi(argv[4]);
    else
        num_threads = 1;
    //std::cout << "Number of args: " << argc << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    std::cout << "Number of multiscale levels: " << level << std::endl;

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

    // begin Otsu filter
    OtsuFilterType::Pointer OtsuFilter = OtsuFilterType::New();
    OtsuFilter->SetInput(reader->GetOutput());

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

    /*
    EightBitImageType::Pointer BDOutputImage = OtsuFilter->GetOutput();
    for(int i = 0; i < 1; i++)
    {
        BinaryDilateFilter->SetInput(BDOutputImage);
        BinaryDilateFilter->Update();
        BDOutputImage = BinaryDilateFilter->GetOutput();
        BDOutputImage->DisconnectPipeline();
    }
    */

    // Apply mask
    MaskFilterType::Pointer MaskFilter = MaskFilterType::New();
    MaskFilter->SetInput1(reader->GetOutput());
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

    /*
    // Multiresolution pyramid filter
    MultiresFilterType::Pointer MultiresFilter = MultiresFilterType::New();
    itk::Array2D<unsigned int> factors(2, 3);
    factors(0, 0) = 4;
    factors(0, 1) = 4;
    factors(0, 2) = 1;
    factors(1, 0) = 2;
    factors(1, 1) = 2;
    factors(1, 2) = 1;

    MultiresFilter->SetInput(MaskFilter->GetOutput());
    MultiresFilter->SetNumberOfThreads(num_threads);
    MultiresFilter->SetSchedule(factors);
    */

    MaskFilter->Update();
    InputImageType::Pointer MultiImage = makeSRGPyramidImage(MaskFilter->GetOutput(), level, num_threads);
    // Apply convex image filter
    ConvexFilterType::Pointer ConvexFilter = ConvexFilterType::New();
    ConvexFilter->SetHeight(240);
    ConvexFilter->SetNumberOfThreads(num_threads);
    //ConvexFilter->FullyConnectedOn();
    //ConvexFilter->SetInput(RGFilter->GetOutput());
    ConvexFilter->SetInput(MultiImage);

    // Rescale image intensity
    RescaleIntensityFilterType::Pointer RescaleIntensityFilter = RescaleIntensityFilterType::New();
    RescaleIntensityFilter->SetOutputMinimum(0);
    RescaleIntensityFilter->SetOutputMaximum(255);
    RescaleIntensityFilter->SetNumberOfThreads(num_threads);
    RescaleIntensityFilter->SetInput(ConvexFilter->GetOutput());

    // Cast image to unsigned pixel type
    DCMToBinaryCastFilterType::Pointer DCMToBinaryCastFilter = DCMToBinaryCastFilterType::New();
    DCMToBinaryCastFilter->SetInput(RescaleIntensityFilter->GetOutput());

    // Apply threshold filter
    EightBitThresholdFilterType::Pointer EightBitThresholdFilter = EightBitThresholdFilterType::New();
    EightBitThresholdFilter->SetOutsideValue(0);
    EightBitThresholdFilter->ThresholdBelow(128);
    EightBitThresholdFilter->SetInput(DCMToBinaryCastFilter->GetOutput());
    EightBitThresholdFilter->SetNumberOfThreads(num_threads);

    // Apply binary threshold filter
    BinaryThresholdFilterType::Pointer BinaryThresholdFilter = BinaryThresholdFilterType::New();
    BinaryThresholdFilter->SetInsideValue(255);
    BinaryThresholdFilter->SetOutsideValue(0);
    BinaryThresholdFilter->SetLowerThreshold(1);
    BinaryThresholdFilter->SetUpperThreshold(255);
    BinaryThresholdFilter->SetInput(EightBitThresholdFilter->GetOutput());

    // Connected component filter
    CCFilterType::Pointer CCFilter = CCFilterType::New();
    CCFilter->SetNumberOfThreads(num_threads);
    CCFilter->SetInput(BinaryThresholdFilter->GetOutput());
    CCFilter->SetMaskImage(BinaryThresholdFilter->GetOutput());
    //CCFilter->FullyConnectedOn();

    // Relabel component filter
    RelabelFilterType::Pointer RelabelFilter = RelabelFilterType::New();
    RelabelFilter->SetInput(CCFilter->GetOutput());
    RelabelFilter->SetNumberOfThreads(num_threads);
    //RelabelFilter->SetMinimumObjectSize(2);

    RelabelFilter->Update();

    /*
    typedef std::vector<unsigned long> SizesInPixelsType;
    const SizesInPixelsType & sizesInPixels = RelabelFilter->GetSizeOfObjectsInPixels();
    SizesInPixelsType::const_iterator sizeItr = sizesInPixels.begin();
    SizesInPixelsType::const_iterator sizeEnd = sizesInPixels.end();
    unsigned int objnum = 0;
    while(sizeItr != sizeEnd)
    {
        std::cout << "Size of object #" << objnum << ": " << *sizeItr << std::endl;
        objnum++;
        sizeItr++;
    }
    */

    printCentroids(RelabelFilter);
    std::cout << "Number of objects detected (all): " << RelabelFilter->GetOriginalNumberOfObjects() << std::endl;
    std::cout << "Number of objects detected (within min size threshold): " << RelabelFilter->GetNumberOfObjects() << std::endl;

    // Write end result of pipeline
    // Set up FileSeriesWriter
    WriterType::Pointer writer = WriterType::New();
    // CHANGE INPUT TO LAST FILTER USED
    writer->SetInput(EightBitThresholdFilter->GetOutput());
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
    std::cout << "Output files successfully written." << std::endl;

    /*
    // Write labeled output files
    // Set up FileSeriesWriter for masks
    LabeledWriterType::Pointer LabeledWriter = LabeledWriterType::New();
    LabeledWriter->SetInput(CCFilter->GetOutput());
    LabeledWriter->SetImageIO(dcmIO);
    const char * LabeledOutputDirectory = argv[3];
    itksys::SystemTools::MakeDirectory(LabeledOutputDirectory);
    nameGenerator->SetOutputDirectory(LabeledOutputDirectory);
    LabeledWriter->SetFileNames(nameGenerator->GetOutputFileNames());
    LabeledWriter->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());

    try
    {
        LabeledWriter->Update();
    }
    catch (itk::ExceptionObject & e)
    {
        std::cerr << "Exception caught in LabeledWriter." << std::endl;
        std::cerr << e << std::endl;
        return -1;
    }
    std::cout << "Relabeled files successfully written." << std::endl;
    */

    return 0;

}
