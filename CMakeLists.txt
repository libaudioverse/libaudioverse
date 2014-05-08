CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(Libaudioverse)
include_directories("${CMAKE_SOURCE_DIR}/include")

#This is LibSNDFile.
SET(LIBSNDFILE_INCLUDE_DIR "C:/Program Files (x86)/Mega-Nerd/libsndfile/include" CACHE PATH
"Directory for Libsndfile headers.")
SET(LIBSNDFILE_LIBRARY_DIR "C:/Program Files (x86)/Mega-Nerd/libsndfile/lib" CACHE PATH
"Libsndfile library directory.")

INCLUDE_DIRECTORIES(${LIBSNDFILE_INCLUDE_DIR})
LINK_DIRECTORIES(${LIBSNDFILE_LIBRARY_DIR})

#For uthash.
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/uthash/src")

#This fixes portaudio.
add_definitions(-DPA_WDMKS_NO_KSGUID_LIB)

#This makes sure that we can link to ourself.
link_directories(${libaudioverse_BINARY_DIR})

#build and setup sdl
add_subdirectory(sdl-mirror)
LINK_DIRECTORIES(${SDL2_BINARY_DIR})
include_directories(${SDL2_SOURCE_DIR})

add_subdirectory(src)
