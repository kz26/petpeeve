// Kevin Zhang
// Bria Connolly
// Last update: June 29, 2011
// DICOM image set is read in. An Otsu Threshold Filter is then run over the images.
// The binary image output from the Otsu Filter is then used as a mask over the original image
// to complete body segmentation.
// A multisclae object recognition approach is then implemented using a series of Gaussian
// smoothing filters with different sigma values.

#include "itkMinimumMaximumImageCalculator.h"
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
#include "itkDerivativeImageFilter.h"
#include "itkZeroCrossingBasedEdgeDetectionImageFilter.h"
#include "itkHConvexImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkLinearInterpolateImageFunction.h"
int main(int argc, char* argv[])
{

if (argc < 3)
{
	std::cerr << "Missing command line arguments" <<  std::endl;
	std::cerr << "Usage: " <<  argv[0] << " input_dcm_directory output_dcm_directory outputmask_dcm_directory" << std::endl;
    return -1;
}	
    typedef unsigned char InputPixelType;               //changed this!!!
	typedef itk::OrientedImage<InputPixelType, 3> TestInputImageType;
    
    
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
    //Want to follow process of filtering then subsampling

    typedef float GaussianPixelType;
    typedef itk::OrientedImage<GaussianPixelType, 3> GaussianImageType;

    typedef itk::CastImageFilter<InputImageType, GaussianImageType> FloatCastFilterType;     //Cast to convert to float pixel type
    FloatCastFilterType::Pointer FloatCastFilter = FloatCastFilterType::New();
    FloatCastFilter->SetInput(MaskFilter->GetOutput());

    typedef itk::SmoothingRecursiveGaussianImageFilter<GaussianImageType, GaussianImageType> GaussianFilterType;    //Smoothing Gaussian Filter
    GaussianFilterType::Pointer GaussianFilter = GaussianFilterType::New();
    GaussianFilter->SetNormalizeAcrossScale(false);
    GaussianFilter->SetInput(FloatCastFilter->GetOutput());
    GaussianFilter->SetSigma(10);

    //Apply the resampling filter
    typedef itk::MinimumMaximumImageCalculator <InputImageType> ImageCalculatorFilterType;      //Compute the maximum and minimum pixel intensity values for the input images
    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(reader->GetOutput());
    signed short min =  imageCalculatorFilter->GetMinimum();
    signed short max =  imageCalculatorFilter->GetMaximum();
    std::cout<<"min "<<min<<std::endl;
    std::cout<<"max "<<max<<std::endl;

    typedef float InternalPixelType;
    typedef unsigned char ResamplePixelType;
    const double factorX=1;                         //Use max and min values from reader to rescale image to orginal size
    const double factorY=1;
    const double factorZ=1;                         //Z direction always equals 1


    typedef itk::OrientedImage<InternalPixelType, 3> InternalImageType;
    typedef itk::OrientedImage<ResamplePixelType, 3> ResampleImageType;

    typedef itk::ResampleImageFilter<GaussianImageType, ResampleImageType> ResampleFilterType;
    ResampleFilterType::Pointer ResampleFilter = ResampleFilterType::New();
    
    typedef itk::IdentityTransform<double, 3> TransformType;    //Only transform on image should be rescaling
    TransformType::Pointer Transformer=TransformType::New();
    Transformer->SetIdentity();
    ResampleFilter->SetTransform(Transformer);

    typedef itk::LinearInterpolateImageFunction<InternalImageType, double>InterpolatorType;          //Specify interpolation algorithm
    InterpolatorType::Pointer Interpolator=InterpolatorType::New();
    ResampleFilter->SetInterpolator(Interpolator);

    ResampleImageType::SpacingType spacing;     //Specify spacing for output image
    const InputImageType::SpacingType& inputSpacing = reader->GetOutput()->GetSpacing();
    spacing[0]=inputSpacing[0]*factorX;
    spacing[1]=inputSpacing[1]*factorY;
    spacing[2]=inputSpacing[2]*factorZ;
    
    ResampleFilter->SetOutputSpacing(spacing);

    ResampleFilter->SetOutputOrigin(reader->GetOutput()->GetOrigin());
    InputImageType::SizeType inputSize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
    typedef InputImageType::SizeType::SizeValueType SizeValueType;

    InputImageType::SizeType size;

    size[0] = static_cast<SizeValueType>(inputSize[0]/factorX);
    size[1] = static_cast<SizeValueType>(inputSize[1]/factorY);
    size[2] = static_cast<SizeValueType>(inputSize[2]/factorZ);

    ResampleFilter->SetSize(size);
    ResampleFilter->SetInput(GaussianFilter->GetOutput());


    //Apply convex image filter
    typedef itk::HConvexImageFilter<GaussianImageType, GaussianImageType> ConvexImageFilterType;
    ConvexImageFilterType::Pointer ConvexImageFilter= ConvexImageFilterType::New();
    
    ConvexImageFilter->SetHeight(100);
    ConvexImageFilter->SetInput(GaussianFilter->GetOutput());

    //Rescale Image Intensity
    typedef itk::RescaleIntensityImageFilter<GaussianImageType,GaussianImageType> RescaleIntensityFilterType;
    RescaleIntensityFilterType::Pointer RescaleIntensityFilter= RescaleIntensityFilterType::New();
    RescaleIntensityFilter->SetOutputMinimum(0);
    RescaleIntensityFilter->SetOutputMaximum(255);
    RescaleIntensityFilter->SetInput(ConvexImageFilter->GetOutput());

    //Cast image to unsigned pixel type
    typedef itk::CastImageFilter<GaussianImageType, Binary3DImageType> DCMToBinaryCast3DFilterType;
    DCMToBinaryCast3DFilterType::Pointer DCMToBinaryCast3D = DCMToBinaryCast3DFilterType::New();
    DCMToBinaryCast3D->SetInput(RescaleIntensityFilter->GetOutput());

    //Apply Thresholding Filter
    typedef itk::ThresholdImageFilter<Binary3DImageType> BinaryThresholdFilterType;
    BinaryThresholdFilterType::Pointer BinaryThresholdFilter=BinaryThresholdFilterType::New();
    BinaryThresholdFilter->SetOutsideValue(0);
    BinaryThresholdFilter->ThresholdBelow(249);
    BinaryThresholdFilter->SetInput(DCMToBinaryCast3D->GetOutput());

/*
    //ERROR IN ATTEMPT TO IMPLEMENT: The number of filenames passed is 355 but 1 were expected
    //Implement Canny Edge Detection as edge localization method
    //Output should be a binary image
    float variance = 0.5;               //define threshold values for filter
    float upperThreshold = 100.0;
    float lowerThreshold = 0.0;
    typedef double CannyPixelType;      
    typedef itk::Image<CannyPixelType, 3> CannyImageType;
    //typedef itk::Image<CannyPixelType, 2> CannyImageType;

    typedef itk::CastImageFilter<GaussianImageType, CannyImageType> DCMToCannyFilterType; //Filter operates on float pixel type so cast the input images
    typedef itk::RescaleIntensityImageFilter<CannyImageType, GaussianImageType> RescaleFilterType;
    typedef itk::CannyEdgeDetectionImageFilter<CannyImageType, CannyImageType> CannyFilterType;

    DCMToCannyFilterType::Pointer CastImage = DCMToCannyFilterType::New();
    RescaleFilterType::Pointer RescaleImage = RescaleFilterType::New();
    CannyFilterType::Pointer CannyFilter = CannyFilterType::New();

    RescaleImage->SetOutputMinimum(0);      //Output of edge filter is 0 or 1
    RescaleImage->SetOutputMaximum(255);

    CastImage->SetInput(GaussianFilter3->GetOutput());

    CannyFilter->SetInput(CastImage->GetOutput());
    CannyFilter->SetVariance(variance);
    CannyFilter->SetUpperThreshold(upperThreshold);
    CannyFilter->SetLowerThreshold(lowerThreshold);

    RescaleImage->SetInput(CannyFilter->GetOutput());
   
    //Then can run nonmaximum supression
    //After that run hysteresis thresholding
   
    //Second Order Derivative Filter
    typedef itk::DerivativeImageFilter<GaussianImageType, InputImageType> DerivativeFilterType;
    DerivativeFilterType::Pointer DerivativeFilter=DerivativeFilterType::New();

    DerivativeFilter->SetOrder(2);
    DerivativeFilter->SetDirection(0);

    DerivativeFilter->SetInput(RescaleImage->GetOutput());

    //Zero Crossing Based Filter
    typedef double EdgeInputPixelType;
    typedef double EdgeOutputPixelType;
    typedef char EdgePixelType;
    typedef itk::OrientedImage<EdgeInputPixelType, 2> EdgeInputImageType;
    typedef itk::OrientedImage<EdgeOutputPixelType, 2> EdgeOutputImageType;
    typedef itk::OrientedImage<EdgePixelType, 2> EdgeImageType;
    typedef itk::ZeroCrossingBasedEdgeDetectionImageFilter<EdgeInputImageType, EdgeOutputImageType> ZeroCrossingEdgeFilterType;
    typedef itk::RescaleIntensityImageFilter<EdgeOutputImageType, EdgeImageType> RescaleImageFilterType;
    typedef itk::CastImageFilter<InputImageType, EdgeInputImageType> InputToEdgeCastFilterType;
    InputToEdgeCastFilterType::Pointer InputToEdgeCast = InputToEdgeCastFilterType::New();
    ZeroCrossingEdgeFilterType::Pointer ZeroCrossingFilter=ZeroCrossingEdgeFilterType::New();
    RescaleImageFilterType::Pointer RescaleFilter=RescaleImageFilterType::New();

    InputToEdgeCast->SetInput(DerivativeFilter->GetOutput());

    ZeroCrossingFilter->SetVariance(5.0);
    ZeroCrossingFilter->SetMaximumError(0.5);
    ZeroCrossingFilter->SetInput(InputToEdgeCast->GetOutput());

    RescaleFilter->SetInput(ZeroCrossingFilter->GetOutput());
    RescaleFilter->SetOutputMinimum(0);
    RescaleFilter->SetOutputMaximum(255);
*/
   //
   //
   // cast ResampleImage to outputImage


    typedef itk::OrientedImage<DCMPixelType, 3> OutputImageType3D;
    typedef itk::CastImageFilter<ResampleImageType, OutputImageType3D> OutputCastFilterType;
  OutputCastFilterType::Pointer OutputCastFilter = OutputCastFilterType::New();
    OutputCastFilter->SetInput(ResampleFilter->GetOutput());
OutputCastFilter->Update();
    std::cout<<"cast good\n";

    // Write end result of pipeline
    // Set up FileSeriesWriter
   // typedef itk::OrientedImage<DCMPixelType, 3> OutputImageType;
   // typedef itk::OrientedImage<ResamplePixelType, 2> OutputImageType;
    //typedef itk::ImageSeriesWriter<Binary3DImageType, OutputImageType> WriterType;
    typedef itk::OrientedImage<DCMPixelType, 2> OutputImageType;
    typedef itk::ImageSeriesWriter<OutputImageType3D, OutputImageType> WriterType;
    //typedef itk::ImageSeriesWriter<ResampleImageType, OutputImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    //writer->SetInput(GaussianFilter->GetOutput());
    //writer->SetInput(ResampleFilter->GetOutput());
    writer->SetImageIO(dcmIO);
    const char * outputDirectory = argv[2];
    itksys::SystemTools::MakeDirectory(outputDirectory);
    nameGenerator->SetOutputDirectory(outputDirectory);
    writer->SetFileNames(nameGenerator->GetOutputFileNames());
    writer->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());

    writer->SetInput(OutputCastFilter->GetOutput());
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

  /* 
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
*/
    return 0;
  
}
