# This is the root ITK CMakeLists file.
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)


PROJECT(petpeeve)

# Find ITK.
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

ADD_EXECUTABLE(petpeeve petpeeve.cxx ext_functions.cxx)
ADD_EXECUTABLE(petpeeve_rg petpeeve_rg.cxx ext_functions.cxx)
SET(CMAKE_CXX_FLAGS "-g")

TARGET_LINK_LIBRARIES(petpeeve ITKCommon ITKIO ITKAlgorithms)
TARGET_LINK_LIBRARIES(petpeeve_rg ITKCommon ITKIO ITKAlgorithms)
