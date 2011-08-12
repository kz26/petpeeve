#ifndef SCCA_H
#define SCCA_H

#include "itkOrientedImage.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

typedef signed short DCMPixelType;
typedef unsigned char EightBitPixelType;
typedef signed short LongPixelType;
typedef float FloatPixelType;

typedef itk::OrientedImage<DCMPixelType, 3> DCMImageType;
typedef itk::OrientedImage<EightBitPixelType, 3> EightBitImageType;

typedef itk::ImageSeriesReader<DCMImageType> ReaderType;
typedef itk::GDCMImageIO ImageIOType;
typedef itk::GDCMSeriesFileNames NamesGeneratorType;

typedef std::vector<std::string> FileNamesContainer;

typedef itk::BinaryThresholdImageFilter<DCMImageType, EightBitImageType> BinaryThresholdFilterType;
typedef itk::ConnectedComponentImageFilter<DCMImageType, DCMImageType, DCMImageType> CCFilterType;
typedef itk::RelabelComponentImageFilter<DCMImageType, DCMImageType> RelabelFilterType;
typedef itk::RescaleIntensityImageFilter<DCMImageType, DCMImageType> RescaleIntensityFilterType;

// File series writer types
typedef itk::OrientedImage<DCMPixelType, 2> OutputImageType;
typedef itk::ImageSeriesWriter<DCMImageType, OutputImageType> WriterType;

#endif

