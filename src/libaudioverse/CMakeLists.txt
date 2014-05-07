add_library(libaudioverse SHARED
nodes.c
graphs.c
streams.c

#specific node types.
nodes/sine.c
)
TARGET_LINK_LIBRARIES(libaudioverse libsndfile-1)
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")