add_executable(convolve convolve.c)
target_link_libraries(convolve xxx)
SET_PROPERTY(TARGET convolve PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")
