/*
 * External functions for petpeeve
 * Kevin Zhang
 * July 8, 2011
 */
#include "itktypes.h"

using namespace std;

int round(float num)
{
    return static_cast<int>(floor(num + 0.5));
}

// imported from 2009_11_06_CandidateFinder_adaptive_best
void printCentroids(RelabelFilterType::Pointer RelabelFilter)
{
    InputImageType::RegionType InputRegion;
    InputImageType::RegionType::IndexType InputStart;

    InputStart[0] = 0;
    InputStart[1] = 0;
    InputStart[2] = 0;

    InputRegion.SetSize(RelabelFilter->GetOutput()->GetRequestedRegion().GetSize() );
    InputRegion.SetIndex(InputStart);

    typedef itk::ImageRegionConstIteratorWithIndex<LongImageType> ConstIteratorWithIndexType;
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
        if(objsizes[i] <= 20)
            std::cout << round(centroids[i][1]) << " " << round(centroids[i][0]) << " " << round(centroids[i][2]) << " " << objsizes[i] << std::endl;
    }

}

InputImageType::Pointer makeSRGPyramidImage(InputImageType::Pointer inputimage, int levels, int threads)
{
    // Set level
    int real_level = min(levels, 3);
    // Set up resample filter
    typedef itk::ResampleImageFilter<InputImageType, InputImageType> ResampleImageFilterType;
    ResampleImageFilterType::Pointer ResampleImageFilter = ResampleImageFilterType::New();

    // Set up affine transform
    typedef itk::IdentityTransform<double, 3> TransformType;
    TransformType::Pointer Transform = TransformType::New();
    Transform->SetIdentity();
    ResampleImageFilter->SetTransform(Transform);

    // Set up interpolator
    typedef itk::LinearInterpolateImageFunction<InputImageType, double> InterpolatorType;
    InterpolatorType::Pointer Interpolator = InterpolatorType::New();
    ResampleImageFilter->SetInterpolator(Interpolator);
    
    // Set parameters
    ResampleImageFilter->SetDefaultPixelValue(0);
    //double spacing[3] = {1.0, 1.0, 1.0};
    const InputImageType::SpacingType origspacing = inputimage->GetSpacing();
    InputImageType::SpacingType newspacing;
    ResampleImageFilter->SetOutputSpacing(newspacing);
    ResampleImageFilter->SetOutputOrigin(inputimage->GetOrigin());
    ResampleImageFilter->SetOutputDirection(inputimage->GetDirection());
    ResampleImageFilter->SetNumberOfThreads(threads);
    const InputImageType::SizeType origsize = inputimage->GetLargestPossibleRegion().GetSize();
    InputImageType::SizeType newsize;
    for (int i = 0; i < 3; i++)
    {
        newsize[i] = origsize[i];
        newspacing[i] = origspacing[i];
    }
    // Set up smoothing recursive Gaussian blur
    RGFilterType::Pointer RGFilter = RGFilterType::New();
    RGFilter->SetNormalizeAcrossScale(false);
    RGFilter->SetNumberOfThreads(threads);
    int starting_sigma = 5;

    InputImageType::Pointer myimage = inputimage;
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

        starting_sigma++;
        //const InputImageType::SizeType mysize = myimage->GetLargestPossibleRegion().GetSize();
        //std::cout << mysize[0] << " " << mysize[1] << " " << mysize[2] << std::endl;
    }
    return myimage;
}

