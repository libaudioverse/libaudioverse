#This always runs which is what we need.
add_custom_target(metadata
COMMAND ${PYTHON_COMMAND} "${CMAKE_SOURCE_DIR}/metadata/metadata.py" "${CMAKE_CURRENT_BINARY_DIR}/metadata.cpp")

#These two drive the building of the default hrtf.
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/default_hrtf.hrtf"
COMMAND ${PYTHON_COMMAND}
ARGS
"${CMAKE_SOURCE_DIR}/scripts/diffuse_mit_hrtf.py"
"${CMAKE_SOURCE_DIR}/diffuse mit kemar dataset"
"${CMAKE_CURRENT_BINARY_DIR}/default_hrtf.hrtf"
DEPENDS
"${CMAKE_SOURCE_DIR}/scripts/diffuse_mit_hrtf.py"
"${CMAKE_SOURCE_DIR}/scripts/hrtf_writer.py"
)

add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/data/default_hrtf.cpp"
COMMAND ${PYTHON_COMMAND}
ARGS
"${CMAKE_SOURCE_DIR}/scripts/convert_to_cpp_file.py" 
"libaudioverse_implementation"
"default_hrtf"
"${CMAKE_CURRENT_BINARY_DIR}/default_hrtf.hrtf"
"${CMAKE_CURRENT_BINARY_DIR}/data/default_hrtf.cpp"
DEPENDS "${CMAKE_SOURCE_DIR}/scripts/convert_to_cpp_file.py"
"${CMAKE_CURRENT_BINARY_DIR}/default_hrtf.hrtf"
)

#We must marke metadata.cpp as generated.
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/metadata.cpp" PROPERTY GENERATED TRUE)

add_library(libaudioverse SHARED
#the core infrastructure.
connections.cpp
node.cpp
buffer.cpp
properties.cpp
initialization.cpp
memory.cpp
simulation.cpp
logging.cpp
planner.cpp
error.cpp
hrtf.cpp
utf8.cpp

file_io/file_reader.cpp
file_io/file_writer.cpp

#classes and functionality for DSp that are stateful, and miscellaneous DSP-related math functions.
dsp/ringmod.cpp
dsp/db.cpp
dsp/euclid.cpp
dsp/hadamard_matrix.cpp
dsp/householder_matrix.cpp
dsp/complex.cpp

#the metadata store
#It appears that CMake dies hard if this isn't an absolute path.
"${CMAKE_CURRENT_BINARY_DIR}/metadata.cpp"

#mathematical kernels for common dsp operations.
kernels/convolution.cpp
kernels/resamplers.cpp
kernels/interleaving.cpp
kernels/adding.cpp
kernels/multiplying.cpp
kernels/dot.cpp

#Like kernels, but stateful.
implementations/iir.cpp
implementations/amplitude_panner.cpp
implementations/delayringbuffer.cpp
implementations/crossfadingdelayline.cpp
implementations/dopplering_delay_line.cpp
implementations/block_convolver.cpp
implementations/fft_convolver.cpp
implementations/biquad.cpp
implementations/interpolated_delay_line.cpp
implementations/nested_allpass_network.cpp
implementations/hrtf_panner.cpp

#specific node types.
nodes/additive_saw.cpp
nodes/additive_square.cpp
nodes/additive_triangle.cpp
nodes/allpass.cpp
nodes/amplitude_panner.cpp
nodes/biquad.cpp
nodes/blit.cpp
nodes/buffer.cpp
nodes/buffer_timeline.cpp
nodes/convolver.cpp
nodes/crossfader.cpp
nodes/crossfading_delay.cpp
nodes/custom.cpp
nodes/dc_blocker.cpp
nodes/dopplering_delay.cpp
nodes/fdn_reverb.cpp
nodes/feedback_delay_network.cpp
nodes/fft_convolver.cpp
nodes/filtered_delay.cpp
nodes/first_order_filter.cpp
nodes/gain.cpp
nodes/graph_listener.cpp
nodes/hard_limiter.cpp
nodes/hrtf.cpp
nodes/iir.cpp
nodes/leaky_integrator.cpp
nodes/multipanner.cpp
nodes/nested_allpass_network.cpp
nodes/noise.cpp
nodes/one_pole_filter.cpp
nodes/panner_bank.cpp
nodes/pull.cpp
nodes/push.cpp
nodes/recorder.cpp
nodes/ringmod.cpp
nodes/sine.cpp
nodes/split_merge.cpp #contains two nodes under the same class.
nodes/three_band_eq.cpp

#automators
automators/automators.cpp
automators/envelope.cpp
automators/linear_ramp.cpp
automators/set.cpp

#audio output using audio_io:
audio_output.cpp
audio_thread.cpp

#the 3D abstraction on top of libaudioverse.
3d/environment.cpp
3d/source.cpp

#c files containing embedded tables and data that don't change.
#The hrtf is generated above.
data/default_hrtf.cpp
)
TARGET_LINK_LIBRARIES(libaudioverse ${libaudioverse_required_libraries})

#Depend on the generation of metadata.
add_dependencies(libaudioverse metadata)

#all artifacts go to the top of the directory.
#this is to make the bindings generator 	sane and predictable.
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
SET_PROPERTY(TARGET libaudioverse PROPERTY LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
SET_PROPERTY(TARGET libaudioverse PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

SET_PROPERTY(TARGET libaudioverse PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
SET_PROPERTY(TARGET libaudioverse PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
SET_PROPERTY(TARGET libaudioverse PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

#We write our compiler flags out for both C and C++ flags.
#The order here needs to match the order in bindings/get_info.py.
string(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_BUILD_TYPE)
file(WRITE "${CMAKE_BINARY_DIR}/compiler_flags.txt" "${CMAKE_BUILD_TYPE}\n")
FILE(APPEND "${CMAKE_BINARY_DIR}/compiler_flags.txt" "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${UPPER_BUILD_TYPE}}\n")
file(APPEND "${CMAKE_BINARY_DIR}/compiler_flags.txt" "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${UPPER_BUILD_TYPE}}\n")
file(APPEND "${CMAKE_BINARY_DIR}/compiler_flags.txt" "${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_${UPPER_BUILD_TYPE}}")

#Docs.
if(${LIBAUDIOVERSE_DEVMODE})
ADD_CUSTOM_TARGET(libaudioverse_docs
COMMAND ${PYTHON_COMMAND} "\"${CMAKE_SOURCE_DIR}/scripts/build_docs.py\""
)
else()
ADD_CUSTOM_TARGET(libaudioverse_docs ALL
COMMAND ${PYTHON_COMMAND} "\"${CMAKE_SOURCE_DIR}/scripts/build_docs.py\""
)
endif()

#Everything below here is installation related.

set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/libaudioverse")

#Install libaudioverse.
install(TARGETS libaudioverse
DESTINATION .)

#Nightly readme.
install(FILES "${CMAKE_SOURCE_DIR}/nightly_instructions.txt"
DESTINATION .
RENAME readme.txt)

install(FILES "${CMAKE_SOURCE_DIR}/license"
DESTINATION .
RENAME license.txt)

#Grab the headers and put them in the include directory.
install(FILES
"${CMAKE_SOURCE_DIR}/include/libaudioverse/libaudioverse.h"
"${CMAKE_SOURCE_DIR}/include/libaudioverse/libaudioverse_properties.h"
"${CMAKE_SOURCE_DIR}/include/libaudioverse/libaudioverse3d.h"
DESTINATION "include/libaudioverse")

#Copy the manual.
install(FILES "${CMAKE_BINARY_DIR}/documentation/libaudioverse_manual.html"
DESTINATION .)

#Grab a copy of Libsndfile.
#Note: this only applies to Windows, and only until we figure out how to statically link Libsndfile.
install(FILES "${LIBSNDFILE_BINARY_DIR}/libsndfile-1.dll"
DESTINATION .)

#Python installation.  Grab the wheel.
install(DIRECTORY "${CMAKE_BINARY_DIR}/bindings/python/dist/"
DESTINATION .)
