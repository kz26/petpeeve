/*
 * External functions
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
    for(int i = 0; i < RelabelFilter->GetNumberOfObjects(); i++)
    {
      int numPixelsInObject = 0;
      int xTotal = 0, yTotal = 0, zTotal = 0;

      for( InputIterator.GoToBegin(); !InputIterator.IsAtEnd(); ++InputIterator)
      {
        if(InputIterator.Get() == i)
        {
            const Binary3DImageType::IndexType index = InputIterator.GetIndex();
            xTotal += index[0];
            yTotal += index[1];
            zTotal += index[2];
            numPixelsInObject++;
        }
      }
      centroids[i][0] = static_cast<float>(xTotal) / static_cast<float>(numPixelsInObject);
      centroids[i][1] = static_cast<float>(yTotal) / static_cast<float>(numPixelsInObject);
      centroids[i][2] = static_cast<float>(zTotal) / static_cast<float>(numPixelsInObject);
    }

    for(int i = 0; i < RelabelFilter->GetNumberOfObjects(); i++)
    {
        std::cout << round(centroids[i][0]) << " " << round(centroids[i][1]) << " " << round(centroids[i][2]) << std::endl;
    }

}

