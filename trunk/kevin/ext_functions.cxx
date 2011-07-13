/*
 * External functions for petpeeve
 * Kevin Zhang
 * July 8, 2011
 */
#include "itktypes.h"

int round(float num)
{
    return static_cast<int>(floor(num + 0.5));
}

// imported from 2009_11_06_CandidateFinder_adaptive_best
void printCentroids(RelabelFilterType::Pointer RelabelFilter)
{
    LongImageType::RegionType InputRegion;
    LongImageType::RegionType::IndexType InputStart;

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
             const Binary3DImageType::IndexType index = InputIterator.GetIndex();
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

