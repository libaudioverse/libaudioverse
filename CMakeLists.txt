cmake_MINIMUM_REQUIRED(VERSION 3.0.0)

#Force release as our default, if the user hasn't overridden.
#Libaudioverse is almost completely useless in debug.
#The project command sets this, if we haven't.
if(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type. Either Debug, Release, RelWithDebInfo, or MinSizeRel." FORCE)
endif()

project(Libaudioverse)

if("$ENV{APPVEYOR}")
set(PYTHON_COMMAND "c:\\python35\\python")
#Are we using the Windows Python launcher?
elseif(${WIN32})
set(PYTHON_COMMAND "py")
set(PYTHON_ARGS "-3")
elseif(UNIX)
set(PYTHON_COMMAND python3)
else()
set(PYTHON_COMMAND python)
endif()
message("Python command is ${PYTHON_COMMAND} with args ${PYTHON_ARGS}")

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

#Libraries CMake has modules for.
find_package(Threads REQUIRED)
if(NOT MSVC)
find_package(Boost REQUIRED
COMPONENTS locale iostreams filesystem thread)
else()
message("You are on MSVC. Skipping boost and assuming it exists.")
message("NOTE: Boost will auto-link needed libraries.")
endif()

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
SET(libaudioverse_required_libraries ${libaudioverse_required_libraries} ${libsndfile_name} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} kissfft audio_io logger_singleton speex_resampler_cpp powercores)
add_subdirectory(src)

#this makes the bindings generation step always run after a Libaudioverse build.
ADD_CUSTOM_TARGET(libaudioverse_bindings ALL
COMMAND ${PYTHON_COMMAND} ${PYTHON_ARGS} "\"${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_bindings.py\""
DEPENDS libaudioverse
)