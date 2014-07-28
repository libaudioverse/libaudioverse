cmake_MINIMUM_REQUIRED(VERSION 2.8)

project(Libaudioverse)
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/lambdatask/include")

#This is LibSNDFile for windows.
#if you've got them somewhere else, you need to change these lines.
if(${MSVC})
SET(LIBSNDFILE_INCLUDE_DIR "C:/Program Files (x86)/Mega-Nerd/libsndfile/include" CACHE PATH
"Directory for Libsndfile headers.")
SET(LIBSNDFILE_LIBRARY_DIR "C:/Program Files (x86)/Mega-Nerd/libsndfile/lib" CACHE PATH
"Libsndfile library directory.")

INCLUDE_DIRECTORIES(${LIBSNDFILE_INCLUDE_DIR})
LINK_DIRECTORIES(${LIBSNDFILE_LIBRARY_DIR})
endif()

#more msvc specific stuff: if on windows, do special handling for portaudio.
if(${MSVC})
#turn off the secure crt warnings, because we're not allowed to use microsoft specific functions.
#Also turn off unsafe stl warnings.  This library likes the unsafe stl.
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

#This fixes portaudio.
#first, disable ksguid.lib.
add_definitions(-DPA_WDMKS_NO_KSGUID_LIB)
#next, disable wasapi, which is apparently very very fragile.
set(PA_USE_WASAPI OFF CACHE BOOL "Note: WASAPI is broken. Leave as off")
set(PA_USE_WDMKS OFF CACHE BOOL "WDMKS is majorly deprecated. Leave as off")
#build and setup portaudio:
add_subdirectory(portaudio)
include_directories("${portaudio_SOURCE_DIR}/include")
endif()

#For linux and mac, we must append -std=c99 and -std=c++99
if(${CMAKE_COMPILER_IS_GNUCC})
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(${CMAKE_COMPILER_IS_GNUCC})
SET(portaudio_name portaudio)
SET(libsndfile_name sndfile)
endif()
if(${MSVC})
set(portaudio_name portaudio_static)
set(libsndfile_name libsndfile-1)
endif()
#include glm
include_directories("${CMAKE_SOURCE_DIR}/glm")

#below here is us.  First, set a variable to all the libraries we need to link with:
SET(libaudioverse_required_libraries ${libsndfile_name} ${portaudio_name})
add_subdirectory(src)
