add_library(libaudioverse SHARED
nodes.c
graphs.c
streams.c
threads.cpp
audio_output.c

#specific node types.
nodes/sine.c
)
TARGET_LINK_LIBRARIES(libaudioverse ${libaudioverse_required_libraries})
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")