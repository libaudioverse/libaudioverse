cmake_MINIMUM_REQUIRED(VERSION 2.8)

project(powercores)

option(POWERCORES_BUILD_TESTS "Whether to build the Powercores tests." ON)

enable_testing()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(src)
