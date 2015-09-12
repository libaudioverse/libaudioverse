cmake_minimum_required(VERSION 3.0)

project(logger_singleton)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(logger_singleton STATIC
src/logger_singleton/logger_singleton.cpp
)

