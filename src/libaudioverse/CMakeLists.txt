add_library(libaudioverse
nodes.c
)
TARGET_LINK_LIBRARIES(libaudioverse libsndfile-1)
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")