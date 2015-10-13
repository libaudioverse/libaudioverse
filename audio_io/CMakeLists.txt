CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(audio_io)

OPTION(AUDIO_IO_BUILD_TESTS "Whether to build tests from audio_io" ON)
OPTION(AUDIO_IO_HANDLED_DEPENDENCIES "Whether or not a parent script handles our dependencies.  Don't touch this, it's for Libaudioverse." OFF)

if(${WIN32})
add_definitions(-DAUDIO_IO_WINDOWS_BACKENDS)
set(AUDIO_IO_NEEDED_LIBRARIES winmm)
endif()

if(NOT ${AUDIO_IO_HANDLED_DEPENDENCIES})
message("Building audio_io in standalone configuration.")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/logger_singleton/include")
add_subdirectory(logger_singleton)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/speex_resampler_cpp/include")
add_subdirectory(speex_resampler_cpp)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/powercores/include")
add_subdirectory(powercores)
endif()

SET(AUDIO_IO_NEEDED_LIBRARIES ${AUDIO_IO_NEEDED_LIBRARIES} speex_resampler_cpp powercores logger_singleton)

SET(AUDIO_IO_FILES
src/audio_io/initialization.cpp
src/audio_io/logging.cpp
src/audio_io/outputs/common_output.cpp
src/audio_io/outputs/factory_creation.cpp
src/audio_io/outputs/latency_predictor.cpp
src/audio_io/outputs/windows/winmm_output.cpp
src/audio_io/outputs/windows/wasapi/factory.cpp
src/audio_io/outputs/windows/wasapi/device.cpp
src/audio_io/format_conversion/mixing_matrices.cpp
src/audio_io/format_conversion/remix.cpp
src/audio_io/format_conversion/sample_format_converter.cpp
src/audio_io/helpers/windows/single_threaded_apartment.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(audio_io STATIC
${AUDIO_IO_FILES}
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
