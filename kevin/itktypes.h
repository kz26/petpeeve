// C++ typedef declarations
#ifndef ITKT_H
#define ITKT_H

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
#include "itkGrayscaleErodeImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkHConvexImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
//#include "itkMultiResolutionPyramidImageFilter.h"
//#include "SmoothingRecursiveGaussianMRP.h"
#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkLinearInterpolateImageFunction.h"

// type definitions
typedef signed short DCMPixelType;
typedef unsigned char EightBitPixelType;
typedef signed short LongPixelType;

typedef itk::OrientedImage<DCMPixelType, 3> InputImageType;
typedef itk::OrientedImage<EightBitPixelType, 3> EightBitImageType;
typedef itk::OrientedImage<LongPixelType, 3> LongImageType;

typedef itk::ImageSeriesReader<InputImageType> ReaderType;
typedef itk::GDCMImageIO ImageIOType;
typedef itk::GDCMSeriesFileNames NamesGeneratorType;

typedef std::vector<std::string> FileNamesContainer;

typedef itk::OtsuThresholdImageFilter<InputImageType, EightBitImageType> OtsuFilterType;
typedef itk::BinaryBallStructuringElement<EightBitPixelType, 3> BBStructuringElementBinType;
typedef itk::BinaryErodeImageFilter<EightBitImageType, EightBitImageType, BBStructuringElementBinType> ErodeFilterType;
typedef itk::BinaryDilateImageFilter<EightBitImageType, EightBitImageType, BBStructuringElementBinType> DilateFilterType;
typedef itk::MaskNegatedImageFilter<InputImageType, EightBitImageType, InputImageType> MaskFilterType;
//typedef itk::MedianImageFilter<InputImageType, InputImageType> MedianFilterType;
typedef itk::SmoothingRecursiveGaussianImageFilter<InputImageType, InputImageType> RGFilterType;
//typedef itk::MultiResolutionPyramidImageFilter<InputImageType, InputImageType> MultiresFilterType;
typedef itk::HConvexImageFilter<InputImageType, InputImageType> ConvexFilterType;
typedef itk::RescaleIntensityImageFilter<InputImageType, InputImageType> RescaleIntensityFilterType;
typedef itk::CastImageFilter<InputImageType, EightBitImageType> DCMToBinaryCastFilterType;
typedef itk::ThresholdImageFilter<EightBitImageType> EightBitThresholdFilterType;
typedef itk::ThresholdImageFilter<InputImageType> ThresholdFilterType;
typedef itk::BinaryThresholdImageFilter<EightBitImageType, EightBitImageType> BinaryThresholdFilterType;
typedef itk::ConnectedComponentImageFilter<EightBitImageType, LongImageType, EightBitImageType> CCFilterType;
typedef itk::RelabelComponentImageFilter<LongImageType, LongImageType> RelabelFilterType;

// File series writer types
typedef itk::OrientedImage<EightBitPixelType, 2> OutputImageType;
typedef itk::OrientedImage<EightBitPixelType, 2> BinaryOutputImageType;
typedef itk::ImageSeriesWriter<EightBitImageType, OutputImageType> WriterType;

// Labeled file series writer types (from RelabelComponentImageFilter)
typedef itk::ImageSeriesWriter<InputImageType, OutputImageType> LabeledWriterType;

/*
// Mask writer types
typedef itk::OrientedImage<EightBitPixelType, 2> MaskOutputImageType;
typedef itk::ImageSeriesWriter<EightBitImageType, MaskOutputImageType> MaskWriterType;
*/

// function definitions
//void printCentroids(RelabelFilterType::Pointer);

#endif
