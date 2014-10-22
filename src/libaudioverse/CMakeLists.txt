add_custom_command(
OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/metadata/metadata.cpp"
COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.py" "${CMAKE_CURRENT_BINARY_DIR}/metadata/metadata.cpp"
DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.t" "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.py" "${CMAKE_CURRENT_SOURCE_DIR}/metadata/metadata.y")

add_library(libaudioverse SHARED
#the core infrastructure.
objects.cpp
properties.cpp
functiontables.cpp
initialization.cpp
termination.cpp
memory.cpp
callbacks.cpp
simulation.cpp
logging.cpp

#utilities: file reading, platform-specific code, stateful filters that can't be represented as kernels, etc:
utils/file_reader.cpp
utils/interleaving.cpp
utils/hrtf.cpp
utils/resampler.cpp

#classes and functionality for DSp that are stateful, and miscellaneous DSP-related math functions.
dsp/biquad.cpp
dsp/ringmod.cpp

#the metadata store
metadata/metadata.cpp

#mathematical kernels for common dsp operations.
kernels/convolution.cpp
kernels/panning.cpp
kernels/resamplers.cpp

#specific node types.
objects/sine.cpp
objects/file.cpp
objects/hard_limiter.cpp
objects/hrtf.cpp
objects/mixer.cpp
objects/attenuator.cpp
objects/delay.cpp
objects/amplitude_panner.cpp
objects/push.cpp
objects/biquad.cpp

#physical outputs, etc.
audio_backends/common.cpp
audio_backends/public.cpp
#audio_backends/portaudio.cpp
#audio_backends/winmm.cpp
audio_backends/openal.cpp

#the 3D abstraction on top of libaudioverse.
3d/world.cpp
3d/sources.cpp
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