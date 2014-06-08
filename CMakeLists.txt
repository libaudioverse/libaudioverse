CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(Libaudioverse)
include_directories("${CMAKE_SOURCE_DIR}/include")

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

#uthash
include_directories("${CMAKE_SOURCE_DIR}/uthash/src")

#more msvc specific stuff: if on windows, do special handling for portaudio.
if(${MSVC})
#This fixes portaudio.
add_definitions(-DPA_WDMKS_NO_KSGUID_LIB)
#build and setup portaudio:
add_subdirectory(portaudio)
include_directories(${portaudio_SOURCE_DIR}/include)
link_directories(${portaudio_BINARY_DIR})
endif()

#For linux and mac, we must append -std=c99 and -std=c++99
if(${CMAKE_COMPILER_IS_GNUCC})
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

#This makes sure that we can link to ourself.
link_directories(${libaudioverse_BINARY_DIR})

if(${CMAKE_COMPILER_IS_GNUCC})
SET(portaudio_name portaudio)
SET(libsndfile_name sndfile)
endif()
if(${MSVC})
set(portaudio_name portaudio_static_x86)
set(libsndfile_name libsndfile-1)
endif()

#below here is us.  First, set a variable to all the libraries we need to link with:
SET(libaudioverse_required_libraries ${libsndfile_name} ${portaudio_name})
add_subdirectory(src)
