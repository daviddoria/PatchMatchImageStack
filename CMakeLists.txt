cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/" ${CMAKE_MODULE_PATH})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(FFTW3 REQUIRED COMPONENTS single) # 'single' means find fftw3f
INCLUDE_DIRECTORIES(${FFTW3_INCLUDE_DIRS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_OPENEXR")


include_directories(${CMAKE_SOURCE_DIR}) # Need this because files in src/ want to include submodule things like "Mask/Mask.h" instead of needing to do "./Mask/Mask.h"

add_subdirectory(Mask)

add_subdirectory(src)
