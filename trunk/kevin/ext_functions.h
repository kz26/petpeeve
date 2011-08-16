// C++ header declarations for ext_functions.cxx
// Kevin Zhang
// 07/12/2011
#ifndef EXTF_H
#define EXTF_H

#include "itktypes.h"

int round(float);
EightBitImageType::Pointer GridThreshold(FloatImageType::Pointer, int);
EightBitImageType::Pointer SliceBySliceThreshold(FloatImageType::Pointer);
int findThreshold(Float2DImageType::Pointer);
int findThreshold3D(FloatImageType::Pointer, FloatImageType::Pointer);
void printHistogram(FloatImageType::Pointer);
void printCentroids(RelabelFilterType::Pointer);
//DCMImageType::Pointer makeSRGPyramidImage(DCMImageType::Pointer, int, int);

#endif
