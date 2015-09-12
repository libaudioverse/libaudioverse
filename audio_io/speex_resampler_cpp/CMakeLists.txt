cmake_minimum_required(VERSION 3.0)

project(speex_resampler_cpp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(speex_resampler_cpp
src/speex_resampler.c
src/resampler_class.cpp
src/resampler_function.cpp
src/exceptions.cpp
)
