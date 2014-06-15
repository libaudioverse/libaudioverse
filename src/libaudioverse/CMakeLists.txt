add_library(libaudioverse SHARED
objects.cpp
nodes.cpp
properties.cpp
functiontables.cpp
initialization.cpp
interleaving.cpp
hrtf.cpp
dspmath.cpp
memory_manager.cpp

#threading related functions and structures
threading/threads.cpp
threading/crossthread_ringbuffer.cpp

#specific node types.
nodes/sine.cpp
nodes/file.cpp
nodes/hard_limiter.cpp
nodes/hrtf.cpp
nodes/mixer.cpp
nodes/attenuator.cpp

#the audio backends and related functionality.
audio_devices/device_negociation.cpp
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
