// Kevin Zhang
// Bria Connolly
// Last update: June 29, 2011
// DICOM image set is read in. An Otsu Threshold Filter is then run over the images.
// The binary image output from the Otsu Filter is then used as a mask over the original image
// to complete body segmentation.
// A multisclae object recognition approach is then implemented using a series of Gaussian
// smoothing filters with different sigma values.

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
#include "itkRescaleIntensityImageFilter.h"
#include "itkCannyEdgeDetectionImageFilter.h"
int main(int argc, char* argv[])
{

if (argc < 3)
{
	std::cerr << "Missing command line arguments" <<  std::endl;
	std::cerr << "Usage: " <<  argv[0] << " input_dcm_directory output_dcm_directory outputmask_dcm_directory" << std::endl;
    return -1;
}	

	typedef signed short DCMPixelType;
    typedef itk::OrientedImage<DCMPixelType, 3> InputImageType;
    typedef itk::ImageSeriesReader<InputImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();

    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer dcmIO = ImageIOType::New();
    reader->SetImageIO(dcmIO);
    typedef itk::GDCMSeriesFileNames NamesGeneratorType;

    NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
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
        std::cerr << "Excpetion caught during file reading." << std::endl;
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
    BBStructuringElementBin.SetRadius(1); 
    BBStructuringElementBin.CreateStructuringElement();
 
    // begin binary dilation filter
    typedef itk::BinaryDilateImageFilter<Binary3DImageType, Binary3DImageType, BBStructuringElementBinType> DilateFilterType;
    DilateFilterType::Pointer BinaryDilateFilter = DilateFilterType::New();
    BinaryDilateFilter->SetKernel(BBStructuringElementBin);
    BinaryDilateFilter->SetInput(OtsuFilter->GetOutput());
    BinaryDilateFilter->SetDilateValue(255);

    Binary3DImageType::Pointer BDOutputImage = OtsuFilter->GetOutput();
    for(int i = 0; i < 1; i++)
    {
        BinaryDilateFilter->SetInput(BDOutputImage);
        BinaryDilateFilter->Update();
        BDOutputImage = BinaryDilateFilter->GetOutput();
        BDOutputImage->DisconnectPipeline();
    }

    BBStructuringElementBin.SetRadius(10); 
    // Begin binary erosion filter
    typedef itk::BinaryErodeImageFilter<Binary3DImageType, Binary3DImageType, BBStructuringElementBinType> ErodeFilterType;
    ErodeFilterType::Pointer BinaryErodeFilter = ErodeFilterType::New();
    BinaryErodeFilter->SetErodeValue(255);
    BinaryErodeFilter->SetKernel(BBStructuringElementBin);
    BinaryErodeFilter->SetInput(BDOutputImage);

    //Mask using output from Binary Dilate Filter
    typedef itk::MaskNegatedImageFilter<InputImageType, Binary3DImageType, InputImageType> MaskFilterType;
    MaskFilterType::Pointer MaskFilter = MaskFilterType::New();
    MaskFilter->SetInput1(reader->GetOutput());
    MaskFilter->SetInput2(BinaryErodeFilter->GetOutput());
 
    //MultiScale Object Recognition
    //Multiple Smoothing Recursive Gaussian Filter on Output from Mask Filter
    typedef itk::OrientedImage<DCMPixelType, 3> GaussianImageType;
    typedef itk::SmoothingRecursiveGaussianImageFilter<InputImageType, GaussianImageType> GaussianFilterType;
    GaussianFilterType::Pointer GaussianFilter = GaussianFilterType::New();

    GaussianFilter->SetNormalizeAcrossScale(false);
    GaussianFilter->SetInput(MaskFilter->GetOutput());
    GaussianFilter->SetSigma(10);
    GaussianFilter->Update();

    typedef itk::SmoothingRecursiveGaussianImageFilter<GaussianImageType, GaussianImageType> GaussianFilterType2;
    GaussianFilterType2::Pointer GaussianFilter2 = GaussianFilterType2::New();

    GaussianFilter2->SetNormalizeAcrossScale(false);
    GaussianFilter2->SetInput(GaussianFilter2->GetOutput());
    GaussianFilter2->SetSigma(50);
    GaussianFilter2->Update();

    typedef itk::SmoothingRecursiveGaussianImageFilter<GaussianImageType, GaussianImageType> GaussianFilterType3;
    GaussianFilterType3::Pointer GaussianFilter3=GaussianFilterType3::New();

    GaussianFilter3->SetNormalizeAcrossScale(false);
    GaussianFilter3->SetInput(GaussianFilter3->GetOutput());
    GaussianFilter3->SetSigma(5);
    GaussianFilter3->Update();

    //Implement Canny Edge Detection as edge localization method
    //Output should be a binary image
    float variance = 2.0;               //define threshold values for filter
    float upperThreshold = 2.0;
    float lowerThreshold = 0.0;
    typedef double CannyPixelType;      
    typedef itk::Image<CannyPixelType, 2> CannyImageType;

    typedef itk::CastImageFilter<GaussianImageType, CannyImageType> DCMToCannyFilterType; //Filter operates on float pixel type so cast the input images
    typedef itk::RescaleIntensityImageFilter<CannyImageType, GaussianImageType> RescaleFilterType;
    typedef itk::CannyEdgeDetectionImageFilter<CannyImageType, CannyImageType> CannyFilterType;

    DCMtoCannyFilterType::Pointer CastImage = DCMtoCannyFilterType::New();
    RescaleFilterType::Pointer RescaleImage = RescaleFilterType::New();
    CannyFilterType::Pointer CannyFilter = CannyFilterType::New();

    RescaleImage->SetOutputMinimum(0);      //Output of edge filter is 0 or 1
    RescaleImage->SetOutputMaximum(255);

    CastImage->SetInput(GaussianFilter3->GetOutput());

    CannyFilter->SetInput(CastImage->GetOutput());
    CannyFilter->SetVariance(variance);
    CannyFilter->SetUpperThreshold(upperThreshold);
    CannyFilter->SetLowerThreshold(lowerThreshold);


    //Then can run nonmaximum supression
    //After that run hysteresis thresholding
    
    // Write end result of pipeline
    // Set up FileSeriesWriter
    typedef itk::OrientedImage<DCMPixelType, 2> OutputImageType;
    typedef itk::ImageSeriesWriter<GaussianImageType, OutputImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(GaussianFilter3->GetOutput());
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
    MaskWriter->SetInput(BinaryErodeFilter->GetOutput());
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
