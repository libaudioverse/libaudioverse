CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(audio_io)

OPTION(AUDIO_IO_BUILD_TESTS "Whether to build tests from audio_io" ON)
OPTION(AUDIO_IO_HANDLED_DEPENDENCIES "Whether or not a parent script handles our dependencies.  Don't touch this, it's for Libaudioverse." OFF)

if(CMAKE_COMPILER_IS_GNUC OR CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()

include(cmake_backend_handler.txt)

if(NOT ${AUDIO_IO_HANDLED_DEPENDENCIES})
message("Building audio_io in standalone configuration.")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/logger_singleton/include")
add_subdirectory(logger_singleton)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/speex_resampler_cpp/include")
add_subdirectory(speex_resampler_cpp)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/powercores/include")
add_subdirectory(powercores)
endif()

if(NOT WIN32)
find_package(Boost REQUIRED thread)
else()
message("audio_io: Assuming boost is present because Win32.")
endif()

SET(AUDIO_IO_NEEDED_LIBRARIES ${AUDIO_IO_NEEDED_LIBRARIES} ${Boost_LIBRARIES} speex_resampler_cpp powercores logger_singleton)

SET(AUDIO_IO_NEEDED_FILES
${AUDIO_IO_NEEDED_FILES}
src/audio_io/initialization.cpp
src/audio_io/logging.cpp
src/audio_io/common/common_output.cpp
src/audio_io/common/factory_creation.cpp
src/audio_io/common/latency_predictor.cpp
src/audio_io/format_conversion/mixing_matrices.cpp
src/audio_io/format_conversion/remix.cpp
src/audio_io/format_conversion/sample_format_converter.cpp
)

if(WIN32)
set(AUDIO_IO_NEEDED_FILES
${AUDIO_IO_NEEDED_FILES}
src/audio_io/helpers/windows/single_threaded_apartment.cpp
)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(audio_io STATIC
${AUDIO_IO_NEEDED_FILES}
)
target_link_libraries(audio_io ${AUDIO_IO_NEEDED_LIBRARIES})

if(${AUDIO_IO_BUILD_TESTS})
add_executable(output
src/tests/output.cpp
)
target_link_libraries(output audio_io ${AUDIO_IO_NEEDED_LIBRARIES})

add_executable(enumeration
src/tests/enumeration.cpp
)
target_link_libraries(enumeration audio_io ${AUDIO_IO_NEEDED_LIBRARIES})

endif()
