// C++ typedef declarations
#ifndef ITKT_H
#define ITKT_H

#include "itkOrientedImage.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkOtsuMultipleThresholdsImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkMaskImageFilter.h"
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
#include "itkHConcaveImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkLaplacianRecursiveGaussianImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkScalarImageToHistogramGenerator.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkExtractImageFilter.h"

#include <algorithm>
#include <vector>

// type definitions
typedef signed short DCMPixelType;
typedef unsigned char EightBitPixelType;
typedef signed short LongPixelType;
typedef float FloatPixelType;

typedef itk::OrientedImage<DCMPixelType, 3> DCMImageType;
typedef itk::OrientedImage<DCMPixelType, 2> DCM2DImageType;
typedef itk::OrientedImage<EightBitPixelType, 3> EightBitImageType;
typedef itk::OrientedImage<EightBitPixelType, 2> EightBit2DImageType;
typedef itk::OrientedImage<LongPixelType, 3> LongImageType;
typedef itk::OrientedImage<FloatPixelType, 3> FloatImageType;
typedef itk::OrientedImage<FloatPixelType, 2> Float2DImageType;

typedef itk::ImageSeriesReader<DCMImageType> ReaderType;
typedef itk::GDCMImageIO ImageIOType;
typedef itk::GDCMSeriesFileNames NamesGeneratorType;

typedef std::vector<std::string> FileNamesContainer;

typedef itk::CastImageFilter<DCMImageType, FloatImageType> DCMToFloatFilterType;
typedef itk::CastImageFilter<FloatImageType, DCMImageType> FloatToDCMFilterType;
typedef itk::CastImageFilter<LongImageType, DCMImageType> LongToDCMFilterType;
typedef itk::CastImageFilter<EightBitImageType, DCMImageType> EightBitToDCMFilterType;
typedef itk::CastImageFilter<FloatImageType, EightBitImageType> FloatToEightBitFilterType;
typedef itk::CastImageFilter<EightBitImageType, FloatImageType> EightBitToFloatFilterType;

typedef itk::OtsuThresholdImageFilter<FloatImageType, EightBitImageType> OtsuFilterType;
typedef itk::BinaryBallStructuringElement<EightBitPixelType, 3> BBStructuringElementBinType;
typedef itk::BinaryErodeImageFilter<EightBitImageType, EightBitImageType, BBStructuringElementBinType> ErodeFilterType;
typedef itk::BinaryDilateImageFilter<EightBitImageType, EightBitImageType, BBStructuringElementBinType> DilateFilterType;
typedef itk::InvertIntensityImageFilter<EightBitImageType, EightBitImageType> InvertIntensityFilterType;
typedef itk::MaskImageFilter<FloatImageType, EightBitImageType, FloatImageType> MaskFilterType;
typedef itk::SmoothingRecursiveGaussianImageFilter<FloatImageType, FloatImageType> RGFilterType;
typedef itk::LaplacianRecursiveGaussianImageFilter<FloatImageType, FloatImageType> LoGFilterType;
typedef itk::HessianRecursiveGaussianImageFilter<FloatImageType, FloatImageType> HessianFilterType;
typedef itk::HConvexImageFilter<DCMImageType, DCMImageType> ConvexFilterType;
typedef itk::HConcaveImageFilter<FloatImageType, FloatImageType> ConcaveFilterType;
typedef itk::RescaleIntensityImageFilter<DCMImageType, DCMImageType> RescaleIntensityFilterType;
typedef itk::RescaleIntensityImageFilter<FloatImageType, FloatImageType> FloatRescaleIntensityFilterType;
typedef itk::CastImageFilter<DCMImageType, EightBitImageType> DCMToBinaryCastFilterType;
typedef itk::ThresholdImageFilter<EightBitImageType> EightBitThresholdFilterType;
typedef itk::ThresholdImageFilter<FloatImageType> ThresholdFilterType;
typedef itk::ThresholdImageFilter<DCMImageType> DCMThresholdFilterType;
typedef itk::BinaryThresholdImageFilter<DCMImageType, EightBitImageType> MaskBTFilterType;
typedef itk::BinaryThresholdImageFilter<DCM2DImageType, EightBit2DImageType> BT2DFilterType;
typedef itk::BinaryThresholdImageFilter<FloatImageType, EightBitImageType> BinaryThresholdFilterType;
typedef itk::BinaryThresholdImageFilter<Float2DImageType, EightBit2DImageType> BinaryThreshold2DFilterType;
typedef itk::BinaryThresholdImageFilter<EightBitImageType, EightBitImageType> EightBitBinaryThresholdFilterType;
typedef itk::ExtractImageFilter<FloatImageType, Float2DImageType> ExtractFilterType;
typedef itk::ExtractImageFilter<Float2DImageType, Float2DImageType> Extract2DFilterType;
typedef itk::ConnectedComponentImageFilter<EightBitImageType, DCMImageType, EightBitImageType> CCFilterType;
typedef itk::ConnectedComponentImageFilter<EightBit2DImageType, DCM2DImageType, EightBit2DImageType> CC2DFilterType;
typedef itk::RelabelComponentImageFilter<DCMImageType, DCMImageType> RelabelFilterType;
typedef itk::RelabelComponentImageFilter<DCM2DImageType, DCM2DImageType> Relabel2DFilterType;

// File series writer types
typedef itk::OrientedImage<DCMPixelType, 2> OutputImageType;
typedef itk::OrientedImage<EightBitPixelType, 2> BinaryOutputImageType;
typedef itk::ImageSeriesWriter<DCMImageType, OutputImageType> WriterType;

/*
// Mask writer types
typedef itk::OrientedImage<EightBitPixelType, 2> MaskOutputImageType;
typedef itk::ImageSeriesWriter<EightBitImageType, MaskOutputImageType> MaskWriterType;
*/

// function definitions
//void printCentroids(RelabelFilterType::Pointer);

#endif
