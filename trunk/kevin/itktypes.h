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
#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkLaplacianRecursiveGaussianImageFilter.h"

#include <algorithm>

// type definitions
typedef signed short DCMPixelType;
typedef unsigned char EightBitPixelType;
typedef signed short LongPixelType;
typedef float FloatPixelType;

typedef itk::OrientedImage<DCMPixelType, 3> DCMImageType;
typedef itk::OrientedImage<EightBitPixelType, 3> EightBitImageType;
typedef itk::OrientedImage<LongPixelType, 3> LongImageType;
typedef itk::OrientedImage<FloatPixelType, 3> FloatImageType;

typedef itk::ImageSeriesReader<DCMImageType> ReaderType;
typedef itk::GDCMImageIO ImageIOType;
typedef itk::GDCMSeriesFileNames NamesGeneratorType;

typedef std::vector<std::string> FileNamesContainer;

typedef itk::CastImageFilter<DCMImageType, FloatImageType> DCMToFloatFilterType;
typedef itk::CastImageFilter<FloatImageType, DCMImageType> FloatToDCMFilterType;
typedef itk::CastImageFilter<LongImageType, DCMImageType> LongToDCMFilterType;
typedef itk::CastImageFilter<EightBitImageType, DCMImageType> EightBitToDCMFilterType;

typedef itk::OtsuThresholdImageFilter<FloatImageType, EightBitImageType> OtsuFilterType;
typedef itk::BinaryBallStructuringElement<EightBitPixelType, 3> BBStructuringElementBinType;
typedef itk::BinaryErodeImageFilter<EightBitImageType, EightBitImageType, BBStructuringElementBinType> ErodeFilterType;
typedef itk::BinaryDilateImageFilter<EightBitImageType, EightBitImageType, BBStructuringElementBinType> DilateFilterType;
typedef itk::MaskNegatedImageFilter<FloatImageType, EightBitImageType, FloatImageType> MaskFilterType;
typedef itk::SmoothingRecursiveGaussianImageFilter<FloatImageType, FloatImageType> RGFilterType;
typedef itk::LaplacianRecursiveGaussianImageFilter<FloatImageType, FloatImageType> LoGFilterType;
typedef itk::HConvexImageFilter<DCMImageType, DCMImageType> ConvexFilterType;
typedef itk::RescaleIntensityImageFilter<DCMImageType, DCMImageType> RescaleIntensityFilterType;
typedef itk::CastImageFilter<DCMImageType, EightBitImageType> DCMToBinaryCastFilterType;
typedef itk::ThresholdImageFilter<EightBitImageType> EightBitThresholdFilterType;
typedef itk::ThresholdImageFilter<FloatImageType> ThresholdFilterType;
typedef itk::BinaryThresholdImageFilter<FloatImageType, EightBitImageType> BinaryThresholdFilterType;
typedef itk::ConnectedComponentImageFilter<EightBitImageType, DCMImageType, EightBitImageType> CCFilterType;
typedef itk::RelabelComponentImageFilter<DCMImageType, DCMImageType> RelabelFilterType;

// File series writer types
typedef itk::OrientedImage<DCMPixelType, 2> OutputImageType;
typedef itk::OrientedImage<EightBitPixelType, 2> BinaryOutputImageType;
typedef itk::ImageSeriesWriter<DCMImageType, OutputImageType> WriterType;

// Labeled file series writer types (from RelabelComponentImageFilter)
typedef itk::ImageSeriesWriter<DCMImageType, OutputImageType> RawWriterType;

/*
// Mask writer types
typedef itk::OrientedImage<EightBitPixelType, 2> MaskOutputImageType;
typedef itk::ImageSeriesWriter<EightBitImageType, MaskOutputImageType> MaskWriterType;
*/

// function definitions
//void printCentroids(RelabelFilterType::Pointer);

#endif
