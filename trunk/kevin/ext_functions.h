// C++ header declarations for ext_functions.cxx
// Kevin Zhang
// 07/12/2011
#ifndef EXTF_H
#define EXTF_H

#include "itktypes.h"

int round(float);
int findThreshold(FloatImageType::Pointer, FloatImageType::Pointer);
void printHistogram(FloatImageType::Pointer);
void printCentroids(RelabelFilterType::Pointer);
//DCMImageType::Pointer makeSRGPyramidImage(DCMImageType::Pointer, int, int);

#endif
