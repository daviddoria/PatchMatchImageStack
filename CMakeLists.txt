cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/" ${CMAKE_MODULE_PATH})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(FFTW3 REQUIRED COMPONENTS single) # 'single' means find fftw3f
INCLUDE_DIRECTORIES(${FFTW3_INCLUDE_DIRS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_OPENEXR")

add_subdirectory(src)
