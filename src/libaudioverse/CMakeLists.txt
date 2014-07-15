	add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/metadata/metadata.cpp"
COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.py" "${CMAKE_CURRENT_BINARY_DIR}/metadata/metadata.cpp"
DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.t" "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.py" "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.y")

add_library(libaudioverse SHARED
objects.cpp
properties.cpp
functiontables.cpp
initialization.cpp
memory.cpp

#utilities: file reading, platform-specific code, etc:
utils/file_reader.cpp
utils/interleaving.cpp
utils/hrtf.cpp
utils/dspmath.cpp

#the metadata store
metadata/metadata.cpp

#mathematical kernels for common dsp operations.
kernels/convolution.cpp

#specific node types.
objects/sine.cpp
objects/file.cpp
objects/hard_limiter.cpp
objects/hrtf.cpp
objects/mixer.cpp
objects/attenuator.cpp
objects/delay.cpp

#the audio backends and related functionality.
audio_devices/device_negotiation.cpp
audio_devices/devices.cpp
audio_devices/portaudio.cpp

#the 3D abstraction on top of libaudioverse.
3d/world.cpp
3d/sources.cpp
)
TARGET_LINK_LIBRARIES(libaudioverse ${libaudioverse_required_libraries})

#all artifacts go to the top of the directory.
#this is to make the testing framework sane and predictable.
SET_PROPERTY(TARGET libaudioverse PROPERTY LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")
SET_PROPERTY(TARGET libaudioverse PROPERTY ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")
