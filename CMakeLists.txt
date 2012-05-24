cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_OPENEXR")

add_subdirectory(src)
