// Automated tumor detection in PET images
// Kevin Zhang
// Bria Connolly
// Last update: 07/12/2011
// DICOM image set is read in. An Otsu Threshold Filter is then run over the images.
// The binary image output from the Otsu Filter is then used as a mask over the original image
// to complete body segmentation.

#include "itktypes.h"
#include "ext_functions.h"
#include "itkLaplacianRecursiveGaussianImageFilter.h"
#include "itkImageLinearConstIteratorWithIndex.h"
#include "itkImageSliceConstIteratorWithIndex.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkCastImageFilter.h"
int main(int argc, char* argv[])
{

    if (argc < 5)
    {
	    std::cerr << "Usage: " <<  argv[0] << " input_dcm_directory output_dcm_directory outputmask_dcm_directory [# threads]" << std::endl;
        return -1;
    }	

    int num_threads;
    if (argc == 5)
        num_threads = atoi(argv[4]);
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

    /*
    Binary3DImageType::Pointer BDOutputImage = OtsuFilter->GetOutput();
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
    MaskFilter->SetInput1(DCMToFloatFilter->GetOutput());
    //MaskFilter->SetInput2(BinaryDilateFilter->GetOutput());
    MaskFilter->SetInput2(BinaryDilateFilter->GetOutput());

    /*
    // Apply recursive Gaussian blur
    RGFilterType::Pointer RGFilter = RGFilterType::New();
    RGFilter->SetNormalizeAcrossScale(false);
    RGFilter->SetSigma(5);
    RGFilter->SetNumberOfThreads(num_threads);
    RGFilter->SetInput(MaskFilter->GetOutput());

    */
 
    /*
    // Multiresolution pyramid filter
    MultiresFilterType::Pointer MultiresFilter = MultiresFilterType::New();
    unsigned int startfactors[3] = {2, 2, 1};
    MultiresFilter->SetInput(MaskFilter->GetOutput());
    MultiresFilter->SetNumberOfThreads(num_threads);
    MultiresFilter->SetStartingShrinkFactors(startfactors);
    */
    /*
    // Median filter
    MedianFilterType::Pointer MedianFilter = MedianFilterType::New();
    InputImageType::SizeType indexRadius;
    indexRadius[0] = 1;
    indexRadius[1] = 1;
    indexRadius[2] = 1;
    MedianFilter->SetRadius(indexRadius);
    MedianFilter->SetNumberOfThreads(num_threads);
    MedianFilter->SetInput(RGFilter->GetOutput());
    */

    /*
    // Apply threshold filter
    ThresholdFilterType::Pointer ThresholdFilter = ThresholdFilterType::New();
    ThresholdFilter->SetOutsideValue(0);
    ThresholdFilter->ThresholdBelow(10000);
    ThresholdFilter->SetInput(RGFilter->GetOutput());
    ThresholdFilter->SetNumberOfThreads(num_threads);
    */

   //Implement Laplacian of Gaussian Filter
    typedef itk::LaplacianRecursiveGaussianImageFilter<InputImageType, EightBitImageType> LaplacianGaussianImageFilterType;
    LaplacianGaussianImageFilterType::Pointer LGImageFilter = LaplacianGaussianImageFilterType::New();
    LGImageFilter->SetInput(MaskFilter->GetOutput());
    LGImageFilter->SetSigma(10);
    LGImageFilter->SetNormalizeAcrossScale(false);


    //Implement Gray Level thresholding
    typedef itk::ThresholdImageFilter<EightBitImageType> GrayThresholdFilterType;
    GrayThresholdFilterType::Pointer GrayThresholdFilter = GrayThresholdFilterType::New();
    GrayThresholdFilter->SetOutsideValue(0);
    GrayThresholdFilter->ThresholdBelow(255);
    //GrayThresholdFilter->ThresholdAbove(255);
    GrayThresholdFilter->SetInput(LGImageFilter->GetOutput());
    
    //Use output from LoG Filter and Thresholding to Mask over segmented images
    typedef itk::MaskNegatedImageFilter<InputImageType, EightBitImageType, InputImageType> MaskFilterType;
    MaskFilterType::Pointer MaskFilter2 = MaskFilterType::New();
    MaskFilter2->SetInput1(MaskFilter->GetOutput());
    MaskFilter2->SetInput2(GrayThresholdFilter->GetOutput());
    
    // Apply convex image filter
    ConvexFilterType::Pointer ConvexFilter = ConvexFilterType::New();
    ConvexFilter->SetHeight(1500);
    ConvexFilter->SetNumberOfThreads(num_threads);
    //ConvexFilter->FullyConnectedOn();
    //ConvexFilter->SetInput(RGFilter->GetOutput());
    ConvexFilter->SetInput(MaskFilter2->GetOutput());
    */

    CCFilterType::Pointer CCFilter = CCFilterType::New();
    CCFilter->SetNumberOfThreads(num_threads);
    CCFilter->SetInput(BinaryThresholdFilter->GetOutput());
    CCFilter->SetMaskImage(BinaryThresholdFilter->GetOutput());
    //CCFilter->FullyConnectedOn();
    
    unsigned int min_object_size = 10;
    // Relabel component filter
    RelabelFilterType::Pointer RelabelFilter = RelabelFilterType::New();
    RelabelFilter->SetInput(CCFilter->GetOutput());
    RelabelFilter->SetNumberOfThreads(num_threads);
    RelabelFilter->SetMinimumObjectSize(2);

    RelabelFilter->Update();

    
    /*
    // Apply threshold filter
    ThresholdFilterType::Pointer ThresholdFilter = ThresholdFilterType::New();
    ThresholdFilter->SetOutsideValue(0);
    ThresholdFilter->ThresholdBelow(128);
    ThresholdFilter->SetInput(DCMToBinaryCastFilter->GetOutput());
    ThresholdFilter->SetNumberOfThreads(num_threads);
    */

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

    // Convert from eight-bit to DCM pixel type
    //EightBitToDCMFilterType::Pointer EightBitToDCMFilter = EightBitToDCMFilterType::New();
    //EightBitToDCMFilter->SetInput(BinaryThresholdFilter->GetOutput());

    // Write end result of pipeline
    // Set up FileSeriesWriter
    WriterType::Pointer writer = WriterType::New();
    // CHANGE INPUT TO LAST FILTER USED
    writer->SetInput(BinaryThresholdFilter->GetOutput());
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
    std::cout << "Files successfully written." << std::endl;

    // Write binary mask files
    // Set up FileSeriesWriter for masks
    MaskWriterType::Pointer MaskWriter = MaskWriterType::New();
    MaskWriter->SetInput(BinaryDilateFilter->GetOutput());
    MaskWriter->SetImageIO(dcmIO);
    const char * MaskOutputDirectory = argv[3];
    itksys::SystemTools::MakeDirectory(MaskOutputDirectory);
    nameGenerator->SetOutputDirectory(MaskOutputDirectory);
    MaskWriter->SetFileNames(nameGenerator->GetOutputFileNames());
    MaskWriter->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());

    try
    {
        MaskWriter->Update();
    }
    catch (itk::ExceptionObject & e)
    {
        std::cerr << "Exception caught in MaskWriter." << std::endl;
        std::cerr << e << std::endl;
        return -1;
    }
    std::cout << "Mask files successfully written." << std::endl;

    return 0;
}
