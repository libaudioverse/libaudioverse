cmake_MINIMUM_REQUIRED(VERSION 3.0.0)

#Force release as our default, if the user hasn't overridden.
#Libaudioverse is almost completely useless in debug.
#The project command sets this, if we haven't.
if(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type. Either Debug, Release, RelWithDebInfo, or MinSizeRel." FORCE)
endif()

project(Libaudioverse)

#Are we using the Windows Python launcher?
if(${WIN32})
set(PYTHON_COMMAND py -3)
else()
set(PYTHON_COMMAND python)
endif()

include_directories("${CMAKE_SOURCE_DIR}/include")

option(LIBAUDIOVERSE_DEVMODE "Whether this is being built for official release. Makes some targets (documentation) optional" ON)

#Which CPU extensions to enable?
option(LIBAUDIOVERSE_USE_SSE2 "Use SSE2" ON)
#this is the required alignment for allocation, a default which is configured in case sse/other processor extensions are disabled.
SET(LIBAUDIOVERSE_MALLOC_ALIGNMENT 1)
if(${LIBAUDIOVERSE_USE_SSE2})
SET(LIBAUDIOVERSE_MALLOC_ALIGNMENT 16)
ENDIF()

#sets up compiler flags for things: sse, vc++ silencing, etc.
#This needs to be first to force MSVC static runtime.
#We declare its options here so that they are advertised to readers of this file.
option(LIBAUDIOVERSE_MSVC_FORCE_STATIC_RUNTIME "Force VC++ to statically link the runtime" ON)
include("cmake_include/compiler_flags.txt")

#Libraries we vendor.
include("vendoring/libsndfile.txt")
include("vendoring/glm.txt")
include("vendoring/kissfft.txt")

#mine. Order matters because of include directories.
include("vendoring/logger_singleton.txt")
include("vendoring/powercores.txt")
include("vendoring/speex_resampler_cpp.txt")
include("vendoring/audio_io.txt")


#All the libraries we need to link with. Platform-specific libraries are set in the include file for compiler flags.
SET(libaudioverse_required_libraries ${libaudioverse_required_libraries} ${libsndfile_name} kissfft audio_io logger_singleton speex_resampler_cpp powercores)
add_subdirectory(src)

#this makes the bindings generation step always run after a Libaudioverse build.
ADD_CUSTOM_TARGET(libaudioverse_bindings ALL
COMMAND ${PYTHON_COMMAND} "\"${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_bindings.py\""
DEPENDS libaudioverse
)

#Bring  in cpack.
set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_FILE_NAME libaudioverse_master)
include(CPack)
