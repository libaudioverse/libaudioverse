cmake_MINIMUM_REQUIRED(VERSION 2.8)

project(Libaudioverse)
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/lambdatask/include")

#Which CPU extensions to enable?
option(LIBAUDIOVERSE_USE_SSE2 "Use SSE2" ON)
#this is the required alignment for allocation, a default which is configured in case sse/other processor extensions are disabled.
SET(LIBAUDIOVERSE_MALLOC_ALIGNMENT 1)
if(${LIBAUDIOVERSE_USE_SSE2})
SET(LIBAUDIOVERSE_MALLOC_ALIGNMENT 16)
ENDIF()

#sets up compiler flags for things: sse, vc++ silencing, etc.
include("cmake_include/compiler_flags.txt")

include("vendoring/openalsoft.txt")
include("vendoring/libsndfile.txt")
include("vendoring/glm.txt")
include("vendoring/kissfft.txt")

#All the libraries we need to link with:
SET(libaudioverse_required_libraries ${libsndfile_name} ${OPENAL_NAME} kissfft)
add_subdirectory(src)

#this makes the bindings generation step always run after a Libaudioverse build.
#ADD_CUSTOM_TARGET(libaudioverse_bindings ALL
#COMMAND python "\"${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_bindings.py\""
#DEPENDS libaudioverse
#)

#This makes documentation run, but only when requested.
ADD_CUSTOM_TARGET(libaudioverse_docs
COMMAND python "\"${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_docs.py\""
)
