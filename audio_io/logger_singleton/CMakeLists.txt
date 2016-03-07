cmake_minimum_required(VERSION 3.0)

project(logger_singleton)

if(CMAKE_COMPILER_IS_GNUC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

find_package(Threads REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(logger_singleton STATIC
src/logger_singleton/logger_singleton.cpp
)
target_link_libraries(logger_singleton ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_logger
src/tests/test_logger.cpp)
target_link_libraries(test_logger logger_singleton)
