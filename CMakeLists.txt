CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(xxx)
include_directories("${CMAKE_SOURCE_DIR}/include")

#This is LibSNDFile.
SET(LIBSNDFILE_INCLUDE_DIR "C:/Program Files (x86)/Mega-Nerd/libsndfile/include" CACHE PATH
"Directory for Libsndfile headers.")
SET(LIBSNDFILE_LIBRARY_DIR "C:/Program Files (x86)/Mega-Nerd/libsndfile/lib" CACHE PATH
"Libsndfile library directory.")

INCLUDE_DIRECTORIES(${LIBSNDFILE_INCLUDE_DIR})
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/portaudio/include")
LINK_DIRECTORIES(${LIBSNDFILE_LIBRARY_DIR})
link_directories("${CMAKE_BINARY_DIR}/portaudio")

#This fixes portaudio.
add_definitions(-DPA_WDMKS_NO_KSGUID_LIB)

#This makes sure that we can link to ourself.
link_directories(${xxx_BINARY_DIR})
add_subdirectory(portaudio)
add_subdirectory(src)
