#This always runs which is what we need.
add_custom_target(metadata
COMMAND python "${CMAKE_SOURCE_DIR}/metadata/metadata.py" "${CMAKE_CURRENT_BINARY_DIR}/metadata.cpp")

#These two drive the building of the default hrtf.
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/default_hrtf.hrtf"
COMMAND python
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
COMMAND python
ARGS
"${CMAKE_SOURCE_DIR}/scripts/convert_to_cpp_file.py" 
"libaudioverse_implementation"
"default_hrtf"
"${CMAKE_CURRENT_BINARY_DIR}/default_hrtf.hrtf"
"${CMAKE_CURRENT_BINARY_DIR}/data/default_hrtf.cpp"
DEPENDS "${CMAKE_SOURCE_DIR}/scripts/convert_to_cpp_file.py"
"${CMAKE_CURRENT_BINARY_DIR}/default_hrtf.hrtf"
)

add_definitions(-DOUTSIDE_SPEEX -DRANDOM_PREFIX=LIBAUDIOVERSE_SPEEX -DFLOATING_POINT)

#We must marke metadata.cpp as generated.
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/metadata.cpp" PROPERTY GENERATED TRUE)

add_library(libaudioverse SHARED
#the core infrastructure.
connections.cpp
node.cpp
buffer.cpp
properties.cpp
functiontables.cpp
initialization.cpp
memory.cpp
events.cpp
simulation.cpp
logging.cpp

#Speex resampler.
speex_resample.c
hrtf.cpp
resampler.cpp

file_io/file_reader.cpp
file_io/file_writer.cpp

#classes and functionality for DSp that are stateful, and miscellaneous DSP-related math functions.
dsp/ringmod.cpp
dsp/iir.cpp

#the metadata store
#It appears that CMake dies hard if this isn't an absolute path.
"${CMAKE_CURRENT_BINARY_DIR}/metadata.cpp"

#mathematical kernels for common dsp operations.
kernels/convolution.cpp
kernels/resamplers.cpp
kernels/interleaving.cpp
kernels/mixing_matrix.cpp
kernels/adding.cpp
kernels/multiplying.cpp

#Like kernels, but stateful.
implementations/panner.cpp
implementations/delayringbuffer.cpp
implementations/crossfadingdelayline.cpp
implementations/dopplering_delay_line.cpp
implementations/feedback_delay_network.cpp

#specific node types.
nodes/sine.cpp
nodes/hard_limiter.cpp
nodes/hrtf.cpp
nodes/crossfading_delay.cpp
nodes/dopplering_delay.cpp
nodes/amplitude_panner.cpp
nodes/push.cpp
nodes/biquad.cpp
nodes/pull.cpp
nodes/graph_listener.cpp
nodes/custom.cpp
nodes/ringmod.cpp
nodes/multipanner.cpp
nodes/feedback_delay_network.cpp
nodes/square.cpp
nodes/noise.cpp
nodes/iir.cpp
nodes/gain.cpp
nodes/split_merge.cpp #contains two nodes under the same class.
nodes/buffer.cpp
nodes/buffer_timeline.cpp
nodes/recorder.cpp

#automators
automators/automators.cpp
automators/envelope.cpp
automators/linear_ramp.cpp
automators/set.cpp

#audio output using audio_io:
audio_output.cpp

#the 3D abstraction on top of libaudioverse.
3d/simpleenvironment.cpp
3d/sources.cpp

#c files containing embedded tables and data that don't change.
data/mixing_matrices.cpp
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