// Kevin Zhang
// Bria Connolly
// Last update: June 29, 2011
// DICOM image set is read in. An Otsu Threshold Filter is then run over the images.
// The binary image output from the Otsu Filter is then used as a mask over the original image
// to complete body segmentation.

#include "itkOrientedImage.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkMaskNegatedImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkOtsuMultipleThresholdsImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkGrayscaleDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkHConvexImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

int main(int argc, char* argv[])
{

    if (argc < 4)
    {
	    std::cerr << "Usage: " <<  argv[0] << " input_dcm_directory output_dcm_directory outputmask_dcm_directory [# threads]" << std::endl;
        return -1;
    }	

    int num_threads;
    if (argc == 5)
        num_threads = atoi(argv[4]);
    else
        num_threads = 1;
    //std::cout << "Number of args: " << argc << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;

	typedef signed short DCMPixelType;
    typedef itk::OrientedImage<DCMPixelType, 3> InputImageType;
    typedef itk::ImageSeriesReader<InputImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();

    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer dcmIO = ImageIOType::New();
    reader->SetImageIO(dcmIO);
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;

    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
    //nameGenerator->SetUseSeriesDetails(true);
    nameGenerator->SetDirectory(argv[1]);

    typedef std::vector<std::string> FileNamesContainer;
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
    typedef unsigned char BinaryPixelType;
    typedef itk::OrientedImage<BinaryPixelType, 3> Binary3DImageType;
    typedef itk::OtsuThresholdImageFilter<InputImageType, Binary3DImageType> OtsuFilterType;
    // typedef itk::OtsuThresholdImageFilter<InputImageType, InputImageType> OtsuFilterType;
    OtsuFilterType::Pointer OtsuFilter = OtsuFilterType::New();
    OtsuFilter->SetInput(reader->GetOutput());

    // Initialize structuring element (binary)
    typedef itk::BinaryBallStructuringElement<BinaryPixelType, 3> BBStructuringElementBinType;
    BBStructuringElementBinType BBStructuringElementBin;
    BBStructuringElementBin.SetRadius(10); 
    BBStructuringElementBin.CreateStructuringElement();

    // begin binary erosion filter
    typedef itk::BinaryErodeImageFilter<Binary3DImageType, Binary3DImageType, BBStructuringElementBinType> ErodeFilterType;
    ErodeFilterType::Pointer BinaryErodeFilter = ErodeFilterType::New();
    BinaryErodeFilter->SetKernel(BBStructuringElementBin);
    BinaryErodeFilter->SetErodeValue(255);
    BinaryErodeFilter->SetInput(OtsuFilter->GetOutput());

    // begin binary dilation filter
    BBStructuringElementBin.SetRadius(1);
    typedef itk::BinaryDilateImageFilter<Binary3DImageType, Binary3DImageType, BBStructuringElementBinType> DilateFilterType;
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
    typedef itk::MaskNegatedImageFilter<InputImageType, Binary3DImageType, InputImageType> MaskFilterType;
    // typedef itk::MaskNegatedImageFilter<InputImageType, InputImageType, InputImageType> MaskFilterType;
    MaskFilterType::Pointer MaskFilter = MaskFilterType::New();
    MaskFilter->SetInput1(reader->GetOutput());
    //MaskFilter->SetInput2(BinaryDilateFilter->GetOutput());
    MaskFilter->SetInput2(BinaryDilateFilter->GetOutput());

    // Apply recursive Gaussian blur
    typedef itk::SmoothingRecursiveGaussianImageFilter<InputImageType, InputImageType> RGFilterType;
    RGFilterType::Pointer RGFilter = RGFilterType::New();
    RGFilter->SetNormalizeAcrossScale(false);
    RGFilter->SetSigma(5);
    RGFilter->SetNumberOfThreads(num_threads);
    RGFilter->SetInput(MaskFilter->GetOutput());

    // Apply convex image filter
    typedef itk::HConvexImageFilter<InputImageType, InputImageType> ConvexFilterType;
    ConvexFilterType::Pointer ConvexFilter = ConvexFilterType::New();
    ConvexFilter->SetHeight(100);
    ConvexFilter->SetNumberOfThreads(num_threads);
    //ConvexFilter->FullyConnectedOn();
    ConvexFilter->SetInput(RGFilter->GetOutput());

    
    // Rescale image intensity 
    typedef itk::RescaleIntensityImageFilter<InputImageType, InputImageType> RescaleIntensityFilterType;
    RescaleIntensityFilterType::Pointer RescaleIntensityFilter = RescaleIntensityFilterType::New();
    RescaleIntensityFilter->SetOutputMinimum(0);
    RescaleIntensityFilter->SetOutputMaximum(255);
    RescaleIntensityFilter->SetNumberOfThreads(num_threads);
    RescaleIntensityFilter->SetInput(ConvexFilter->GetOutput());
    
    // Cast image to unsigned pixel type
    typedef itk::CastImageFilter<InputImageType, Binary3DImageType> DCMToBinaryCastFilterType;
    DCMToBinaryCastFilterType::Pointer DCMToBinaryCastFilter = DCMToBinaryCastFilterType::New();
    DCMToBinaryCastFilter->SetInput(RescaleIntensityFilter->GetOutput());

    /*
    // Apply threshold filter
    typedef itk::ThresholdImageFilter<Binary3DImageType> BinaryThresholdFilterType;
    BinaryThresholdFilterType::Pointer BinaryThresholdFilter = BinaryThresholdFilterType::New();
    BinaryThresholdFilter->SetOutsideValue(0);
    BinaryThresholdFilter->ThresholdBelow(128);
    BinaryThresholdFilter->SetInput(DCMToBinaryCastFilter->GetOutput());
    BinaryThresholdFilter->SetNumberOfThreads(num_threads);
    */

    // Write end result of pipeline
    // Set up FileSeriesWriter
    typedef itk::OrientedImage<DCMPixelType, 2> OutputImageType;
    typedef itk::OrientedImage<BinaryPixelType, 2> BinaryOutputImageType;
    typedef itk::ImageSeriesWriter<Binary3DImageType, BinaryOutputImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    // CHANGE INPUT TO LAST FILTER USED
    writer->SetInput(DCMToBinaryCastFilter->GetOutput());
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
    std::cout << "Files successfully written." << std::endl;

    // Write binary mask files
    // Set up FileSeriesWriter for masks
    typedef itk::OrientedImage<BinaryPixelType, 2> MaskOutputImageType;
    typedef itk::ImageSeriesWriter<Binary3DImageType, MaskOutputImageType> MaskWriterType;
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
