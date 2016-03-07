cmake_minimum_required(VERSION 3.0)

project(speex_resampler_cpp)


if(CMAKE_COMPILER_IS_GNUC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(speex_resampler_cpp
src/speex_resampler.c
src/resampler_class.cpp
src/resampler_function.cpp
src/exceptions.cpp
)
