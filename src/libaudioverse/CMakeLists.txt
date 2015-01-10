add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/metadata/metadata.cpp"
COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.py" "${CMAKE_CURRENT_BINARY_DIR}/metadata/metadata.cpp"
DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.t" "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.py" "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.y")

add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/data/default_hrtf.cpp"
COMMAND python "${CMAKE_SOURCE_DIR}/scripts/convert_to_cpp_file.py" 
"default_hrtf"
"${CMAKE_SOURCE_DIR}/default_hrtf.hrtf"
"${CMAKE_CURRENT_BINARY_DIR}/data/default_hrtf.cpp"
DEPENDS "${CMAKE_SOURCE_DIR}/scripts/convert_to_cpp_file.py"
"${CMAKE_SOURCE_DIR}/default_hrtf.hrtf")

add_library(libaudioverse SHARED
#the core infrastructure.
objects.cpp
properties.cpp
functiontables.cpp
initialization.cpp
termination.cpp
memory.cpp
events.cpp
simulation.cpp
logging.cpp

#utilities: file reading, platform-specific code, stateful filters that can't be represented as kernels, etc:
utils/file_reader.cpp
utils/hrtf.cpp
utils/resampler.cpp

#classes and functionality for DSp that are stateful, and miscellaneous DSP-related math functions.
dsp/ringmod.cpp
dsp/iir.cpp

#the metadata store
metadata/metadata.cpp

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
implementations/feedback_delay_network.cpp

#specific node types.
objects/sine.cpp
objects/file.cpp
objects/hard_limiter.cpp
objects/hrtf.cpp
objects/mixer.cpp
objects/delay.cpp
objects/amplitude_panner.cpp
objects/push.cpp
objects/biquad.cpp
objects/pull.cpp
objects/graph_listener.cpp
objects/custom.cpp
objects/ringmod.cpp
objects/multipanner.cpp
objects/feedback_delay_network.cpp
objects/multifile.cpp
objects/square.cpp
objects/noise.cpp

#physical outputs, etc.
audio_backends/common.cpp
audio_backends/public.cpp
audio_backends/winmm.cpp
audio_backends/openal.cpp

#the 3D abstraction on top of libaudioverse.
3d/simpleenvironment.cpp
3d/sources.cpp

#c files containing embedded tables and data that don't change.
data/mixing_matrices.cpp
data/default_hrtf.cpp
)
TARGET_LINK_LIBRARIES(libaudioverse ${libaudioverse_required_libraries})

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