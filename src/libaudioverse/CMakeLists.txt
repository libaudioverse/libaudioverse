add_library(libaudioverse SHARED
nodes.c
properties.c
graphs.c
streams.c
audio_output.c
interleaving.c
hrtf.c

#data structures (non-thread):
tables.c

#threading related functions and structures
threading/threads.cpp
threading/crossthread_ringbuffer.c

#specific node types.
nodes/sine.c
nodes/file.c
)
TARGET_LINK_LIBRARIES(libaudioverse ${libaudioverse_required_libraries})
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")