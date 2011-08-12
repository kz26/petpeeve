// Kevin Zhang
// Performs connected component / relabel on a DICOM series
#include "simplecca.h"

int main(int argc, char* argv[])
{

    if (argc != 3)
    {   
        std::cerr << "Usage: " <<  argv[0] << " input_dcm_directory output_dir" << std::endl;
        return -1; 
    }   

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

    /*
    BinaryThresholdFilterType::Pointer BinaryThresholdFilter = BinaryThresholdFilterType::New();
    BinaryThresholdFilter->SetOutsideValue(0);
    BinaryThresholdFilter->SetInsideValue(255);
    BinaryThresholdFilter->SetLowerThreshold(-16000);
    BinaryThresholdFilter->SetUpperThreshold(16000);
    BinaryThresholdFilter->SetInput(reader->GetOutput());
    */

    CCFilterType::Pointer CCFilter = CCFilterType::New();
    CCFilter->SetInput(reader->GetOutput());
    CCFilter->SetMaskImage(reader->GetOutput());

    // Relabel component filter
    RelabelFilterType::Pointer RelabelFilter = RelabelFilterType::New();
    RelabelFilter->SetInput(CCFilter->GetOutput());
   
    // Rescale image intensity
    RescaleIntensityFilterType::Pointer RescaleIntensityFilter = RescaleIntensityFilterType::New();
    RescaleIntensityFilter->SetOutputMinimum(0);
    RescaleIntensityFilter->SetOutputMaximum(1000);
    RescaleIntensityFilter->SetInput(RelabelFilter->GetOutput());

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

    return 0;
}

