add_library(libaudioverse SHARED
objects.c
nodes.c
properties.c
graphs.c
audio_output.c
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
)
TARGET_LINK_LIBRARIES(libaudioverse ${libaudioverse_required_libraries})
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")