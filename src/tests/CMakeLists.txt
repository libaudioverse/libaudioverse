macro(test name)
add_executable(${name} ${name}.c)
target_link_libraries(${name} libaudioverse portaudio_static_x86)
SET_PROPERTY(TARGET ${name} PROPERTY RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/tests")
endmacro()

test(test_convolve)
test(test_portaudio)