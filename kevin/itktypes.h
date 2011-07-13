// C++ header declarations for external functions
#ifndef EXTF_H
#define EXTF_H

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

// type definitions
typedef signed short DCMPixelType;
typedef unsigned char BinaryPixelType;
typedef unsigned long LongPixelType;

typedef itk::OrientedImage<DCMPixelType, 3> InputImageType;
typedef itk::OrientedImage<BinaryPixelType, 3> Binary3DImageType;
typedef itk::OrientedImage<LongPixelType, 3> LongImageType;

typedef itk::ImageSeriesReader<InputImageType> ReaderType;
typedef itk::GDCMImageIO ImageIOType;
typedef itk::GDCMSeriesFileNames NamesGeneratorType;

typedef std::vector<std::string> FileNamesContainer;

typedef itk::OtsuThresholdImageFilter<InputImageType, Binary3DImageType> OtsuFilterType;
typedef itk::BinaryBallStructuringElement<BinaryPixelType, 3> BBStructuringElementBinType;
typedef itk::BinaryErodeImageFilter<Binary3DImageType, Binary3DImageType, BBStructuringElementBinType> ErodeFilterType;
typedef itk::BinaryDilateImageFilter<Binary3DImageType, Binary3DImageType, BBStructuringElementBinType> DilateFilterType;
typedef itk::MaskNegatedImageFilter<InputImageType, Binary3DImageType, InputImageType> MaskFilterType;
typedef itk::MedianImageFilter<InputImageType, InputImageType> MedianFilterType;
typedef itk::SmoothingRecursiveGaussianImageFilter<InputImageType, InputImageType> RGFilterType;
typedef itk::HConvexImageFilter<InputImageType, InputImageType> ConvexFilterType;
typedef itk::RescaleIntensityImageFilter<InputImageType, InputImageType> RescaleIntensityFilterType;
typedef itk::CastImageFilter<InputImageType, Binary3DImageType> DCMToBinaryCastFilterType;
typedef itk::ThresholdImageFilter<Binary3DImageType> ThresholdFilterType;
typedef itk::BinaryThresholdImageFilter<Binary3DImageType, Binary3DImageType> BinaryThresholdFilterType;
typedef itk::ConnectedComponentImageFilter<Binary3DImageType, LongImageType, Binary3DImageType> CCFilterType;
typedef itk::RelabelComponentImageFilter<LongImageType, LongImageType> RelabelFilterType;

// File series writer types
typedef itk::OrientedImage<DCMPixelType, 2> OutputImageType;
typedef itk::OrientedImage<BinaryPixelType, 2> BinaryOutputImageType;
typedef itk::ImageSeriesWriter<Binary3DImageType, OutputImageType> WriterType;

// Mask writer types
typedef itk::OrientedImage<BinaryPixelType, 2> MaskOutputImageType;
typedef itk::ImageSeriesWriter<Binary3DImageType, MaskOutputImageType> MaskWriterType;

// function definitions
void printCentroids(RelabelFilterType::Pointer);

#endif
