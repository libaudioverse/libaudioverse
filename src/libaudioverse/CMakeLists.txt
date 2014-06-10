add_library(libaudioverse SHARED
objects.c
nodes.c
properties.c
graphs.c
functiontables.c
initialization.c
interleaving.c
hrtf.c
dspmath.c
memory_manager.c

#data structures (non-thread):
tables.c

#threading related functions and structures
threading/threads.cpp
threading/crossthread_ringbuffer.c

#specific node types.
nodes/sine.c
nodes/file.c
nodes/hrtf.c
nodes/mixer.c
nodes/attenuator.c

#the audio backends and related functionality.
audio_devices/portaudio.c

#the 3D abstraction on top of libaudioverse.
)
TARGET_LINK_LIBRARIES(libaudioverse ${libaudioverse_required_libraries})

#all artifacts go to the top of the directory.
#this is to make the testing framework sane and predictable.
SET_PROPERTY(TARGET libaudioverse PROPERTY LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")
SET_PROPERTY(TARGET libaudioverse PROPERTY ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")
