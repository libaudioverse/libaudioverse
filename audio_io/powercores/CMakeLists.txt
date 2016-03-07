cmake_MINIMUM_REQUIRED(VERSION 2.8)

project(powercores)

option(POWERCORES_BUILD_TESTS "Whether to build the Powercores tests." ON)

if(CMAKE_COMPILER_IS_GNUC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
endif()

find_package(Threads REQUIRED)

enable_testing()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(src)
