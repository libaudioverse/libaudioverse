add_executable(convolve main.c convolution_kernel.c)
TARGET_LINK_LIBRARIES(convolve libsndfile-1)
SET_PROPERTY(TARGET convolve PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}")