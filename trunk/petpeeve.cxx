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
        std::cerr << "Excpetion caught during file reading." << std::endl;
        std::cerr << e << std::endl;
        return -1;
    }   

    /*
    // Initialize structuring element (DCM)
    typedef itk::BinaryBallStructuringElement<DCMPixelType, 3> BBStructuringElementDCMType;
    BBStructuringElementDCMType BBStructuringElementDCM;
    BBStructuringElementDCM.SetRadius(10); // 3x3 neighborhood filter
    BBStructuringElementDCM.CreateStructuringElement();
    
    //Run DICOM images through Grayscale Dilation Filter
    typedef itk::GrayscaleDilateImageFilter<InputImageType, InputImageType, BBStructuringElementDCMType> GrayscaleDilateFilterType;
    GrayscaleDilateFilterType::Pointer GrayscaleDilateFilter = GrayscaleDilateFilterType::New();
    GrayscaleDilateFilter->SetKernel(BBStructuringElementDCM);
    GrayscaleDilateFilter->SetInput(reader->GetOutput());
    */

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
    // typedef itk::BinaryDilateImageFilter<InputImageType, InputImageType, BBStructuringElementType> DilateFilterType;
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
    // begin binary erosion filter
    typedef itk::BinaryErodeImageFilter<Binary3DImageType, Binary3DImageType, BBStructuringElementBinType> ErodeFilterType;
    ErodeFilterType::Pointer BinaryErodeFilter = ErodeFilterType::New();
    BinaryErodeFilter->SetErodeValue(255);
    BinaryErodeFilter->SetKernel(BBStructuringElementBin);
    BinaryErodeFilter->SetInput(BDOutputImage);

    //Mask using output from Binary Dilate Filter
    typedef itk::MaskNegatedImageFilter<InputImageType, Binary3DImageType, InputImageType> MaskFilterType;
    // typedef itk::MaskNegatedImageFilter<InputImageType, InputImageType, InputImageType> MaskFilterType;
    MaskFilterType::Pointer MaskFilter = MaskFilterType::New();
    MaskFilter->SetInput1(reader->GetOutput());
    //MaskFilter->SetInput2(BinaryDilateFilter->GetOutput());
    MaskFilter->SetInput2(BinaryErodeFilter->GetOutput());

    // Write end result of pipeline
    // Set up FileSeriesWriter
    typedef itk::OrientedImage<DCMPixelType, 2> OutputImageType;
    typedef itk::ImageSeriesWriter<InputImageType, OutputImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(MaskFilter->GetOutput());
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
