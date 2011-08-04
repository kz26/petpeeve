/*
 * External functions for petpeeve
 * Kevin Zhang
 * July 8, 2011
 */
#include "itktypes.h"
#include "ext_functions.h"

using namespace std;

int round(float num)
{
    return static_cast<int>(floor(num + 0.5));
}

float mean(std::vector<float> vals)
{
    float sum = 0;
    for(int i = 0; i < vals.size(); i++)
        sum += vals[i];
    return sum / vals.size();
}
float findMax(FloatImageType::Pointer inputimg) // finds the maximum pixel value in an image
{
    typedef itk::MinimumMaximumImageCalculator<FloatImageType> MinMaxFilterType;
    MinMaxFilterType::Pointer MinMaxFilter = MinMaxFilterType::New();
    MinMaxFilter->SetImage(inputimg);
    MinMaxFilter->ComputeMaximum();
    return MinMaxFilter->GetMaximum();
}

// thresholds a 3D input image slice by slice (with different thresholds for each slice)
// outputs a 3D input image
EightBitImageType::Pointer SliceBySliceThreshold(FloatImageType::Pointer inputimg)
{
    // Create output image, using same parameters as input image
    EightBitImageType::Pointer outputimg = EightBitImageType::New();
    EightBitImageType::RegionType outputregion;
    outputregion.SetIndex(inputimg->GetRequestedRegion().GetIndex());
    outputregion.SetSize(inputimg->GetRequestedRegion().GetSize());
    outputimg->SetRegions(outputregion);
    outputimg->Allocate();

    // Get number of slices
    FloatImageType::SizeType size = inputimg->GetRequestedRegion().GetSize();
    int numslices = size[2];
    size[2] = 0; // Extract filter will collapse in z dimension

    typedef itk::ImageRegionConstIteratorWithIndex<EightBit2DImageType> ImageRegion2DIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<EightBitImageType> ImageRegionIteratorType;
    for(int i = 0; i < numslices; i++)
    {
        ExtractFilterType::Pointer ExtractFilter = ExtractFilterType::New();
        FloatImageType::RegionType::IndexType index = inputimg->GetRequestedRegion().GetIndex();
        index[2] = i;
        FloatImageType::RegionType ExtractionRegion;
        ExtractionRegion.SetSize(size);
        ExtractionRegion.SetIndex(index);

        ExtractFilter->SetInput(inputimg);
        ExtractFilter->SetExtractionRegion(ExtractionRegion);
        ExtractFilter->Update();
        
        int slicethres = 1.65 * findThreshold(ExtractFilter->GetOutput());
        BinaryThreshold2DFilterType::Pointer BTFilter = BinaryThreshold2DFilterType::New();
        BTFilter->SetInsideValue(255);
        BTFilter->SetOutsideValue(0);
        BTFilter->SetLowerThreshold(-65536);
        BTFilter->SetUpperThreshold(max(slicethres, -65535));
        BTFilter->SetInput(ExtractFilter->GetOutput());
        std::cerr << "Threshold for image " << i << ": " << slicethres << std::endl;
        BTFilter->Update();

        Float2DImageType::RegionType::IndexType slice_index;
        slice_index[0] = 0;
        slice_index[1] = 0;

        Float2DImageType::SizeType slice_size = BTFilter->GetOutput()->GetRequestedRegion().GetSize();
        Float2DImageType::RegionType slice_region;
        slice_region.SetIndex(slice_index);
        slice_region.SetSize(slice_size);

        EightBitImageType::RegionType::IndexType output_slice_index;
        output_slice_index[0] = 0;
        output_slice_index[1] = 0;
        output_slice_index[2] = i;

        EightBitImageType::SizeType output_slice_size = inputimg->GetRequestedRegion().GetSize();
        output_slice_size[2] = 0;

        EightBitImageType::RegionType output_region;
        output_region.SetIndex(output_slice_index);
        output_region.SetSize(output_slice_size);

        ImageRegion2DIteratorType input_iterator(BTFilter->GetOutput(), slice_region);
        ImageRegionIteratorType output_iterator(outputimg, output_region);

        for(input_iterator.GoToBegin(), output_iterator.GoToBegin(); !input_iterator.IsAtEnd(); ++input_iterator, ++output_iterator) 
        {
            output_iterator.Set(input_iterator.Get());
        }
    }
    return outputimg;
}

int findThreshold(Float2DImageType::Pointer inputimg) // find threshold for a 2D slice
{
    std::vector<float> pixvals;

    Float2DImageType::RegionType InputRegion;
    Float2DImageType::RegionType::IndexType InputStart;
    InputStart[0] = 0;
    InputStart[1] = 0;
    InputRegion.SetSize(inputimg->GetRequestedRegion().GetSize());
    InputRegion.SetIndex(InputStart);

    typedef itk::ImageRegionConstIteratorWithIndex<Float2DImageType> ConstIteratorWithIndexType;
    ConstIteratorWithIndexType InputIterator(inputimg, InputRegion);

    for(InputIterator.GoToBegin(); !InputIterator.IsAtEnd(); ++InputIterator)
    {
        if(InputIterator.Get() != 0)
            pixvals.push_back(InputIterator.Get());
    }
    return round(mean(pixvals));
}

// imported from 2009_11_06_CandidateFinder_adaptive_best
void printCentroids(RelabelFilterType::Pointer RelabelFilter)
{
    DCMImageType::RegionType InputRegion;
    DCMImageType::RegionType::IndexType InputStart;

    InputStart[0] = 0;
    InputStart[1] = 0;
    InputStart[2] = 0;

    InputRegion.SetSize(RelabelFilter->GetOutput()->GetRequestedRegion().GetSize() );
    InputRegion.SetIndex(InputStart);

    typedef itk::ImageRegionConstIteratorWithIndex<DCMImageType> ConstIteratorWithIndexType;
    ConstIteratorWithIndexType InputIterator(RelabelFilter->GetOutput(), InputRegion);

    float centroids[RelabelFilter->GetNumberOfObjects()][3]; // array to store centroid coordinates
    int objsizes[RelabelFilter->GetNumberOfObjects()]; // array to store object sizes
    for(int i = 0; i < RelabelFilter->GetNumberOfObjects(); i++)
    {
        int numPixelsInObject = 0;
        int xTotal = 0, yTotal = 0, zTotal = 0;

        for( InputIterator.GoToBegin(); !InputIterator.IsAtEnd(); ++InputIterator)
        {
            if(InputIterator.Get() == i + 1) // need to skip pixel value 0 for background
            {
             const LongImageType::IndexType index = InputIterator.GetIndex();
             xTotal += index[0];
             yTotal += index[1];
             zTotal += index[2];
             numPixelsInObject++;
            } 
        }
            objsizes[i] = numPixelsInObject;
            centroids[i][0] = static_cast<float>(xTotal) / static_cast<float>(numPixelsInObject);
            centroids[i][1] = static_cast<float>(yTotal) / static_cast<float>(numPixelsInObject);
            centroids[i][2] = static_cast<float>(zTotal) / static_cast<float>(numPixelsInObject);
    }

    for(int i = 0; i < RelabelFilter->GetNumberOfObjects(); i++)
    {
        // i[0] = col, i[1] = row, i[2] = slice according to the ITK docs
        // i[1], i[0], i[2] row, col, slice will be considered the standard output format
        //if(objsizes[i] <= 20)
            std::cout << round(centroids[i][1]) << " " << round(centroids[i][0]) << " " << round(centroids[i][2]) << " " << objsizes[i] << std::endl;
    }

}

void printHistogram(FloatImageType::Pointer inputimage)
{
    typedef itk::Statistics::ScalarImageToHistogramGenerator<FloatImageType > HistogramGeneratorType;
    HistogramGeneratorType::Pointer HistogramGenerator = HistogramGeneratorType::New();
    HistogramGenerator->SetInput(inputimage);
    HistogramGenerator->SetNumberOfBins(100);
    HistogramGenerator->SetMarginalScale(10);
    HistogramGenerator->SetHistogramMin(-500);
    HistogramGenerator->SetHistogramMax(500);
    HistogramGenerator->Compute();

    typedef HistogramGeneratorType::HistogramType HistogramType;
    const HistogramType * histogram = HistogramGenerator->GetOutput();

    const unsigned int histogramSize = histogram->Size();
    std::cerr << "Histogram size " << histogramSize << std::endl;

    unsigned int bin;
    for( bin=0; bin < histogramSize; bin++ )
    {
        std::cerr << "bin = " << bin << " frequency = ";
        std::cerr << histogram->GetFrequency( bin, 0 ) << std::endl;
    }
}
/*
FloatImageType::Pointer makeSRGPyramidImage(FloatImageType::Pointer inputimage, int levels, int threads)
{
    // Set level
    int real_level = min(levels, 3);
    // Set up resample filter
    typedef itk::ResampleImageFilter<FloatImageType, FloatImageType> ResampleImageFilterType;
    ResampleImageFilterType::Pointer ResampleImageFilter = ResampleImageFilterType::New();

    // Set up affine transform
    typedef itk::IdentityTransform<double, 3> TransformType;
    TransformType::Pointer Transform = TransformType::New();
    Transform->SetIdentity();
    ResampleImageFilter->SetTransform(Transform);

    // Set up interpolator
    typedef itk::LinearInterpolateImageFunction<FloatImageType, double> InterpolatorType;
    InterpolatorType::Pointer Interpolator = InterpolatorType::New();
    ResampleImageFilter->SetInterpolator(Interpolator);
    
    // Set parameters
    ResampleImageFilter->SetDefaultPixelValue(0);
    //double spacing[3] = {1.0, 1.0, 1.0};
    const FloatImageType::SpacingType origspacing = inputimage->GetSpacing();
    FloatImageType::SpacingType newspacing;
    ResampleImageFilter->SetOutputSpacing(newspacing);
    ResampleImageFilter->SetOutputOrigin(inputimage->GetOrigin());
    ResampleImageFilter->SetOutputDirection(inputimage->GetDirection());
    ResampleImageFilter->SetNumberOfThreads(threads);
    const FloatImageType::SizeType origsize = inputimage->GetLargestPossibleRegion().GetSize();
    FloatImageType::SizeType newsize;
    for (int i = 0; i < 3; i++)
    {
        newsize[i] = origsize[i];
        newspacing[i] = origspacing[i];
    }
    // Set up smoothing recursive Gaussian blur
    RGFilterType::Pointer RGFilter = RGFilterType::New();
    RGFilter->SetNormalizeAcrossScale(false);
    RGFilter->SetNumberOfThreads(threads);
    int starting_sigma = 6;

    FloatImageType::Pointer myimage = inputimage;
    for(int i = 0; i < real_level; i++)
    {
        RGFilter->SetSigma(starting_sigma);
        RGFilter->SetInput(myimage);
        for (int j = 0; j < 2; j++)
        {
            newsize[j] = round(static_cast<float>(newsize[j]) * 0.75);
            newspacing[j] = newspacing[j] / 0.75;
        }
        //std::cout << newsize[0] << " " << newsize[1] << " " << newsize[2] << std::endl;
        ResampleImageFilter->SetSize(newsize);
        ResampleImageFilter->SetOutputSpacing(newspacing);
        ResampleImageFilter->SetInput(RGFilter->GetOutput());
        ResampleImageFilter->UpdateLargestPossibleRegion();
        myimage = ResampleImageFilter->GetOutput();
        myimage->DisconnectPipeline();

        //starting_sigma += 2;
        //const FloatImageType::SizeType mysize = myimage->GetLargestPossibleRegion().GetSize();
        //std::cout << mysize[0] << " " << mysize[1] << " " << mysize[2] << std::endl;
    }
    return myimage;
}

*/
