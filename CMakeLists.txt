cmake_MINIMUM_REQUIRED(VERSION 2.8)

project(Libaudioverse)
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/lambdatask/include")

#this is the required alignment for allocation, a default which is configured in case sse/other processor extensions are disabled.
SET(LIBAUDIOVERSE_MALLOC_ALIGNMENT 1)

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

if(${MSVC})
#turn off the secure crt warnings, because we're not allowed to use microsoft specific functions.
#Also turn off unsafe stl warnings.  This library likes the unsafe stl.
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
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

#include and build OpenAL-Soft.
#this makes OpenALSoft build statically.
add_definitions(-DAL_LIBTYPE_STATIC)
set(LIBTYPE STATIC)	
add_subdirectory("OpenAL-Soft")
include_directories("OpenAL-Soft/include")
SET(OPENAL_NAME openalsoft_camlorn)

#below here is us.  First our macros.
add_definitions(-DLIBAUDIOVERSE_MALLOC_ALIGNMENT=${LIBAUDIOVERSE_MALLOC_ALIGNMENT})

#Second, set a variable to all the libraries we need to link with:
SET(libaudioverse_required_libraries ${libsndfile_name} ${OPENAL_NAME})
add_subdirectory(src)

#this makes the bindings generation step always run after a Libaudioverse build.
ADD_CUSTOM_TARGET(libaudioverse_bindings ALL
COMMAND python "\"${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_bindings.py\""
DEPENDS libaudioverse
)

#This makes documentation run, but only when requested.
ADD_CUSTOM_TARGET(libaudioverse_docs
COMMAND python "\"${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_docs.py\""
)
