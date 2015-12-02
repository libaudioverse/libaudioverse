/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/**The public interface to Libaudioverse.*/

/*Forward-declares all Libaudioverse types.*/

typedef int LavError;
typedef int LavHandle;
typedef void (*LavParameterlessCallback)(LavHandle nodeHandle, void* userdata);

/**Does whatever is appropriate on a given platform to expose a Libaudioverse function publically.*/
#ifdef _MSC_VER
#define DLL_PUBLIC_ATTR __declspec(dllexport)
#endif
#ifndef DLL_PUBLIC_ATTR
#define DLL_PUBLIC_ATTR
#endif
#ifdef __cplusplus
#define Lav_PUBLIC_FUNCTION extern "C" DLL_PUBLIC_ATTR
#define EXTERN_FUNCTION extern "C"
#else
#define Lav_PUBLIC_FUNCTION extern DLL_PUBLIC_ATTR
#define EXTERN_FUNCTION
#endif

/*This block takes effect if this header is being preprocessed for the binding generators, turning off whatever weird thing we need for the build.*/
#ifdef IS_BINDING
//gets rid of macro redefinition warnings.
#undef Lav_PUBLIC_FUNCTION
#define Lav_PUBLIC_FUNCTION
#undef EXTERN_FUNCTION
#define EXTERN_FUNCTION
#endif

enum Lav_ERRORS {
	Lav_ERROR_NONE, //everything is OK.
	Lav_ERROR_UNKNOWN, //We know something has gone wrong, but can't figure out what.
	Lav_ERROR_TYPE_MISMATCH, //Tried to get/set something with the wrong type, i.e. properties.
	Lav_ERROR_INVALID_PROPERTY, //one of the functions taking a slot got passed an invalid number.
	Lav_ERROR_NULL_POINTER, //you passed a NULL pointer into something that shouldn't have it.
	Lav_ERROR_MEMORY, //a memory problem which probably isn't the fault of the application.
	Lav_ERROR_INVALID_HANDLE,
	Lav_ERROR_RANGE, //out of range function parameter.
	Lav_ERROR_CANNOT_INIT_AUDIO, //We couldn't initialize the audio library or open a device
	Lav_ERROR_FILE, //error to do with files.
	Lav_ERROR_FILE_NOT_FOUND, //specifically, we couldn't find a file.

	Lav_ERROR_HRTF_INVALID,

	Lav_ERROR_CANNOT_CROSS_SIMULATIONS, 

	Lav_ERROR_CAUSES_CYCLE,
	Lav_ERROR_PROPERTY_IS_READ_ONLY,

	Lav_ERROR_OVERLAPPING_AUTOMATORS,

	Lav_ERROR_CANNOT_CONNECT_TO_PROPERTY,
	Lav_ERROR_BUFFER_IN_USE,
	
	Lav_ERROR_INTERNAL= 999,
};

enum Lav_PROPERTY_TYPES {
	Lav_PROPERTYTYPE_INT,
	Lav_PROPERTYTYPE_FLOAT,
	Lav_PROPERTYTYPE_DOUBLE,
	Lav_PROPERTYTYPE_STRING,
	Lav_PROPERTYTYPE_FLOAT3,
	Lav_PROPERTYTYPE_FLOAT6,
	Lav_PROPERTYTYPE_FLOAT_ARRAY,
	Lav_PROPERTYTYPE_INT_ARRAY,
	Lav_PROPERTYTYPE_BUFFER,
};

//Libaudioverse object types.
enum Lav_OBJECT_TYPES {
	Lav_OBJTYPE_SIMULATION,
	Lav_OBJTYPE_BUFFER,

	//Nodes from here.
	Lav_OBJTYPE_GENERIC_NODE, //this is not something you should ever see outside the library, and basically means none.
	Lav_OBJTYPE_ENVIRONMENT_NODE,
	Lav_OBJTYPE_SOURCE_NODE,

	Lav_OBJTYPE_HRTF_NODE,
	Lav_OBJTYPE_SINE_NODE,
	Lav_OBJTYPE_HARD_LIMITER_NODE,
	Lav_OBJTYPE_CROSSFADING_DELAY_NODE,
	Lav_OBJTYPE_DOPPLERING_DELAY_NODE,
	Lav_OBJTYPE_AMPLITUDE_PANNER_NODE,
	Lav_OBJTYPE_PUSH_NODE,
	Lav_OBJTYPE_BIQUAD_NODE,
	Lav_OBJTYPE_PULL_NODE,
	Lav_OBJTYPE_GRAPH_LISTENER_NODE,
	Lav_OBJTYPE_CUSTOM_NODE,
	Lav_OBJTYPE_RINGMOD_NODE,
	Lav_OBJTYPE_MULTIPANNER_NODE,
	Lav_OBJTYPE_FEEDBACK_DELAY_NETWORK_NODE,

	Lav_OBJTYPE_ADDITIVE_SQUARE_NODE,
	Lav_OBJTYPE_ADDITIVE_TRIANGLE_NODE,
	Lav_OBJTYPE_ADDITIVE_SAW_NODE,
	Lav_OBJTYPE_NOISE_NODE,
	Lav_OBJTYPE_IIR_NODE,
	Lav_OBJTYPE_GAIN_NODE,
	Lav_OBJTYPE_CHANNEL_SPLITTER_NODE,
	Lav_OBJTYPE_CHANNEL_MERGER_NODE,
	Lav_OBJTYPE_BUFFER_NODE,
	Lav_OBJTYPE_BUFFER_TIMELINE_NODE,
	Lav_OBJTYPE_RECORDER_NODE,
	Lav_OBJTYPE_CONVOLVER_NODE,
	Lav_OBJTYPE_FFT_CONVOLVER_NODE,
	Lav_OBJTYPE_THREE_BAND_EQ_NODE,
	Lav_OBJTYPE_FILTERED_DELAY_NODE,
	Lav_OBJTYPE_PANNER_BANK_NODE,
	Lav_OBJTYPE_CROSSFADER_NODE,
	Lav_OBJTYPE_ONE_POLE_FILTER_NODE,
	Lav_OBJTYPE_FIRST_ORDER_FILTER_NODE,
	Lav_OBJTYPE_ALLPASS_NODE,
	Lav_OBJTYPE_NESTED_ALLPASS_NETWORK_NODE,
	Lav_OBJTYPE_FDN_REVERB_NODE,
	Lav_OBJTYPE_BLIT_NODE,
	Lav_OBJTYPE_DC_BLOCKER_NODE,
	Lav_OBJTYPE_LEAKY_INTEGRATOR_NODE,
};

/**Node states.*/
enum Lav_NODE_STATES {
	Lav_NODESTATE_PAUSED,
	Lav_NODESTATE_PLAYING,
	Lav_NODESTATE_ALWAYS_PLAYING,
};

/**Logging levels.*/
enum Lav_LOGGING_LEVELS {
	Lav_LOGGING_LEVEL_CRITICAL = 10,
	Lav_LOGGING_LEVEL_INFO = 20,
	Lav_LOGGING_LEVEL_DEBUG = 30,
	Lav_LOGGING_LEVEL_OFF = 40,
};

/**Initialize Libaudioverse.*/
Lav_PUBLIC_FUNCTION LavError Lav_initialize();
/**Shuts down the library.

After a call to this function, any Libaudioverse handles are invalid.
So too are any pointers that Libaudioverse gave you, of any form.*/
Lav_PUBLIC_FUNCTION LavError Lav_shutdown();
Lav_PUBLIC_FUNCTION LavError Lav_isInitialized(int* destination);

/**Query the thread's current error.
Pointers are valid until the next time an error happens on this thread.*/
Lav_PUBLIC_FUNCTION LavError Lav_errorGetMessage(const char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_errorGetFile(const char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_errorGetLine(int* destination);

/**Free any pointer that libaudioverse gives you.  If something goes wrong, namely that the pointer isn't from Libaudioverse in the first place, this tries to fail gracefully and give you an error, but don't rely on this.*/
Lav_PUBLIC_FUNCTION LavError Lav_free(void* ptr);

/**Handle refcounts.*/
Lav_PUBLIC_FUNCTION LavError Lav_handleIncRef(LavHandle handle);
Lav_PUBLIC_FUNCTION LavError Lav_handleDecRef(LavHandle handle);
/**Every handle has a first external access bit which is cleared by this function.
This is to avoid an issue with bindings that need to know if they should increment counts.*/
Lav_PUBLIC_FUNCTION LavError Lav_handleGetAndClearFirstAccess(LavHandle handle, int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_handleGetRefCount(LavHandle handle, int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_handleGetType(LavHandle handle, int* destination);


/**Configure and query logging.

These functions may be used before library initialization, the intent being that you can get initialization logs.  These functions, like all other functions, cannot be used after library termination.*/
typedef void (*LavLoggingCallback)(int level, const char* message);
Lav_PUBLIC_FUNCTION LavError Lav_setLoggingCallback(LavLoggingCallback cb);
Lav_PUBLIC_FUNCTION LavError Lav_getLoggingCallback(LavLoggingCallback* destination);
Lav_PUBLIC_FUNCTION LavError Lav_setLoggingLevel(int level);
Lav_PUBLIC_FUNCTION LavError Lav_getLoggingLevel(int* destination);

/**Configure the handle destroyed callback.  Also may be used before initialization.
This exists only for bindings.  If you use this in your C program, you may have design issues.*/
typedef void (*LavHandleDestroyedCallback)(LavHandle which);
Lav_PUBLIC_FUNCTION LavError Lav_setHandleDestroyedCallback(LavHandleDestroyedCallback cb);

//devices...
/**
Index -1 is special.  Any attempts to query about index -1 will fail.
Opening a device on index -1 requests that the default system audio device be used.  In addition, however, index -1 will follow the default system audio device when the backend supports it.

For audio output devices, valid channel counts are as follows:
1- mono
2- stereo
6- 5.1 surround
8- 7.1 surround

Any channel count that is not one of the above produces undefined behavior, should that channel count be used with the audio backend.

Channel orders for the output node are as follows:
1- mono
2- left, right
6- front left, front right, front center, lfe, back left, back right
8-front left, front right, center, lfe, back left, back right, side left, side right

Internal mixing matrices will handle conversions to and from other formats to the output format. It is suggested that the user is queried for the preferred format, as not all supported APIs are capable of determining system defaults appropriately.  In the event that a backend cannot determine a default, it will suggest 2-channel stereo; this provides a good default for most applications and will be upconverted as needed.
*/
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetCount(unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetName(unsigned int index, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetChannels(unsigned int index, unsigned int* destination);

/**Create the simulation.*/
Lav_PUBLIC_FUNCTION LavError Lav_createSimulation(unsigned int sr, unsigned int blockSize, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlockSize(LavHandle simulationHandle, int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlock(LavHandle simulationHandle, unsigned int channels, int mayApplyMixingMatrix, float* buffer);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetSr(LavHandle simulationHandle, int* destination);

/**Set or clear the output device.*/
Lav_PUBLIC_FUNCTION LavError Lav_simulationSetOutputDevice(LavHandle simulationHandle, int index, int channels, float minLatency, float startLatency, float maxLatency);
Lav_PUBLIC_FUNCTION LavError Lav_simulationClearOutputDevice(LavHandle simulationHandle);

/**Lock/unlock the simulation.*/
Lav_PUBLIC_FUNCTION LavError Lav_simulationLock(LavHandle simulationHandle);
Lav_PUBLIC_FUNCTION LavError Lav_simulationUnlock(LavHandle simulationHandle);

/**The block callback.
This can be used in some situations for precise timing.
This callback is called with a handle to the simulation and a double.  The double is the time in seconds since the beginning of the block at which this callback was first called.
For audio output devices, this callback is called in the mixing thread.  If it blocks, audio stops. Don't.
It is safe to call Libaudioverse from this callback.
To clear, use null as the callback.*/
typedef void (*LavBlockCallback)(LavHandle handle, double time, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_simulationSetBlockCallback(LavHandle simulationHandle, LavBlockCallback callback, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_simulationWriteFile(LavHandle simulationHandle, const char* path, int channels, double duration, int mayApplyMixingMatrix);

Lav_PUBLIC_FUNCTION LavError Lav_simulationSetThreads(LavHandle simulationHandle, int threads);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetThreads(LavHandle simulationHandle, int* destination);

/**Buffers.
Buffers are chunks of audio data from any source.  A variety of nodes to work with buffers exist.*/
Lav_PUBLIC_FUNCTION LavError Lav_createBuffer(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferGetSimulation(LavHandle bufferHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromFile(LavHandle bufferHandle, const char* path);
Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromArray(LavHandle bufferHandle, int sr, int channels, int frames, float* data);
Lav_PUBLIC_FUNCTION LavError Lav_bufferNormalize(LavHandle bufferHandle);
Lav_PUBLIC_FUNCTION LavError Lav_bufferGetDuration(LavHandle bufferHandle, float* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferGetLengthInSamples(LavHandle bufferHandle, int* destination);

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetSimulation(LavHandle nodeHandle, LavHandle* destination);
/**Connect two nodes.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeConnect(LavHandle nodeHandle, int output, LavHandle destHandle, int input);
/**Connect the specified output to the simulation.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeConnectSimulation(LavHandle nodeHandle, int output);
/**Connect a node's output to a properety of another node.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeConnectProperty(LavHandle nodeHandle, int output, LavHandle otherHandle, int slot);
Lav_PUBLIC_FUNCTION LavError Lav_nodeDisconnect(LavHandle nodeHandle, int output, LavHandle otherHandle, int input);
Lav_PUBLIC_FUNCTION LavError Lav_nodeIsolate(LavHandle nodeHandle);

/**Query maximum number of inputs and outputs.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputConnectionCount(LavHandle nodeHandle, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetOutputConnectionCount(LavHandle nodeHandle, unsigned int* destination);

/**Resets a property to its default value, for any type.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeResetProperty(LavHandle nodeHandle, int propertyIndex);

/**Property getters and setters.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetIntProperty(LavHandle nodeHandle, int propertyIndex, int value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloatProperty(LavHandle nodeHandle, int propertyIndex, float value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetDoubleProperty(LavHandle nodeHandle, int propertyIndex, double value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetStringProperty(LavHandle nodeHandle, int propertyIndex, char* value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat3Property(LavHandle nodeHandle, int propertyIndex, float v1, float v2, float v3);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat6Property(LavHandle nodeHandle, int propertyIndex, float v1, float v2, float v3, float v4, float v5, float v6);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntProperty(LavHandle nodeHandle, int propertyIndex, int *destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatProperty(LavHandle nodeHandle, int propertyIndex, float *destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoubleProperty(LavHandle nodeHandle, int propertyIndex, double *destination);
//Note: allocates memory for you, that should be freed with Lav_free.  This is a convenience for those using higher level languages, and to avoid having to make second queries for length.
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetStringProperty(LavHandle nodeHandle, int propertyIndex, const char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat3Property(LavHandle nodeHandle, int propertyIndex, float* destination1, float* destination2, float* destination3);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat6Property(LavHandle nodeHandle, int propertyIndex, float* destinationV1, float* destinationV2, float* destinationV3, float* destinationV4, float* destinationV5, float* destinationV6);

/**Query property ranges. These are set only by internal code.  Float3 and Float6 are effectively rangeless: specifically what a range is for those is very undefined and specific to the node in question.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntPropertyRange(LavHandle nodeHandle, int propertyIndex, int* destinationMin, int* destinationMax);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatPropertyRange(LavHandle nodeHandle, int propertyIndex, float* destinationMin, float* destinationMax);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoublePropertyRange(LavHandle nodeHandle, int propertyIndex, double* destinationMin, double* destinationMax);


Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyName(LavHandle nodeHandle, int propertyIndex, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyType(LavHandle nodeHandle, int propertyIndex, int* destination);
/**Properties with dynamic ranges may change the endpoints of their range at any time and for any reason.
This is primarily used by bindings, but may be useful to user interface developers.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyHasDynamicRange(LavHandle nodeHandle, int propertyIndex, int* destination);

/**Array properties.
An array property does not have a range. Instead, it has a minimum and maximum supported length.
Note that these provide functions for setting parts of the array: the property can store any size of 1-dimensional array.

In order to make reading sane, you can only read one value at a time. Since Libaudioverse never exposes internal memory to the public user, reading multiple values would require wasting huge amounts of ram.
*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceFloatArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int length, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReadFloatArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int index, float* destination);
Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteFloatArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int start, unsigned int stop, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatArrayPropertyLength(LavHandle nodeHandle, int propertyIndex, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceIntArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int length, int* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReadIntArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int index, int* destination);
Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteIntArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int start, unsigned int stop, int* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntArrayPropertyLength(LavHandle nodeHandle, int propertyIndex, int* destination);

//applies to eather array type.
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetArrayPropertyLengthRange(LavHandle nodeHandle, int propertyIndex, unsigned int* destinationMin, unsigned int* destinationMax);

//buffer properties.
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetBufferProperty(LavHandle nodeHandle, int propertyIndex, LavHandle value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetBufferProperty(LavHandle nodeHandle, int propertyIndex, LavHandle* destination);

/**Automators.
These apply only to float and double properties. Times are relative to "now" in node time, not simulation time.
If setting up complex timelines, do it inside an atomic block.
*/
//Clears all automation after time t.
Lav_PUBLIC_FUNCTION LavError Lav_automationCancelAutomators(LavHandle nodeHandle, int propertyIndex, double time);
//Linear ramp to value starting after the last event and ending at time t.
Lav_PUBLIC_FUNCTION LavError Lav_automationLinearRampToValue(LavHandle nodeHandle, int slot, double time, double value);
Lav_PUBLIC_FUNCTION LavError Lav_automationSet(LavHandle nodeHandle, int slot, double time, double value);
Lav_PUBLIC_FUNCTION LavError Lav_automationEnvelope(LavHandle nodeHandle, int slot, double time, double duration, int valuesLength, double *values);

/**Performs the node-specific reset operation.

This does not reset properties, merely internal histories and the like.  Specifically what this means depends on the node; see the manual.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeReset(LavHandle nodeHandle);

//creators for different objject types.
//also see libaudioverse3d.h.

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createAdditiveSquareNode(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createAdditiveTriangleNode(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createAdditiveSawNode(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createNoiseNode(LavHandle simulationHandle, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfNode(LavHandle simulationHandle, const char* hrtfPath, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterNode(LavHandle simulationHandle, int channels, LavHandle *destination);

Lav_PUBLIC_FUNCTION LavError Lav_createCrossfadingDelayNode(LavHandle simulationHandle, float maxDelay, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createDoppleringDelayNode(LavHandle simulationHandle, float maxDelay, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerNode(LavHandle simulationHandle, LavHandle* destination);
//can set the standard channel map for 2, 6, and 8 channels. Other values cause Lav_ERROR_RANGE.
Lav_PUBLIC_FUNCTION LavError Lav_amplitudePannerNodeConfigureStandardMap(LavHandle nodeHandle, unsigned int channels);

Lav_PUBLIC_FUNCTION LavError Lav_createMultipannerNode(LavHandle simulationHandle, char* hrtfPath, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createPushNode(LavHandle simulationHandle, unsigned int sr, unsigned int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_pushNodeFeed(LavHandle nodeHandle, unsigned int length, float* frames);
Lav_PUBLIC_FUNCTION LavError Lav_pushNodeSetLowCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_pushNodeSetUnderrunCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);


Lav_PUBLIC_FUNCTION LavError Lav_createBiquadNode(LavHandle simulationHandle, unsigned int channels, LavHandle* destination);

//pull node:
typedef void (*LavPullNodeAudioCallback)(LavHandle nodeHandle, int frames, int channels, float* buffer, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_createPullNode(LavHandle simulationHandle, unsigned int sr, unsigned int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_pullNodeSetAudioCallback(LavHandle nodeHandle, LavPullNodeAudioCallback callback, void* userdata);

//graph listeners: a way to intercept the graph.
typedef void (*LavGraphListenerNodeListeningCallback)(LavHandle nodeHandle, unsigned int frames, unsigned int channels, float* buffer, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_createGraphListenerNode(LavHandle simulationHandle, unsigned int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_graphListenerNodeSetListeningCallback(LavHandle nodeHandle, LavGraphListenerNodeListeningCallback callback, void* userdata);

//custom nodes.
//the callback does the processing if set, otherwise outputs are zeroed.
typedef void (*LavCustomNodeProcessingCallback)(LavHandle nodeHandle, unsigned int frames, unsigned int numInputs, float** inputs, unsigned int numOutputs, float** outputs, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_createCustomNode(LavHandle simulationHandle, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs, unsigned int channelsPerOutput, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_customNodeSetProcessingCallback(LavHandle nodeHandle, LavCustomNodeProcessingCallback callback, void* userdata);

Lav_PUBLIC_FUNCTION LavError Lav_createRingmodNode(LavHandle simulationHandle, LavHandle* destination);

//these are for feedback delay networks.
//Warning: this is one of if not the single most complex nodes in Libaudioverse.
Lav_PUBLIC_FUNCTION LavError Lav_createFeedbackDelayNetworkNode(LavHandle simulationHandle, float maxDelay, int channels, LavHandle* destination);

//implements iir filters.
Lav_PUBLIC_FUNCTION LavError Lav_createIirNode(LavHandle simulationHandle, int channels, LavHandle* destination);
/*Set the IIR coefficients.
The numerator can be anything, but the denominator must have a nonzero first coefficient, and you must use both.
For a filter without a denominator, use the FIR node-among other things, FIR filters can use floats, whereas this uses doubles internally.*/
Lav_PUBLIC_FUNCTION LavError Lav_iirNodeSetCoefficients(LavHandle nodeHandle, int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory);

//has 1 input and 1 output with channels channels. Passes the input through to the output unchanged. Intended for global volume and similar.
Lav_PUBLIC_FUNCTION LavError Lav_createGainNode(LavHandle simulationHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createChannelSplitterNode(LavHandle simulationHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createChannelMergerNode(LavHandle simulationHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createBufferNode(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferNodeSetEndCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);

Lav_PUBLIC_FUNCTION LavError Lav_createBufferTimelineNode(LavHandle simulationHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferTimelineNodeScheduleBuffer(LavHandle nodeHandle, LavHandle bufferHandle, double time, float pitchBend);

Lav_PUBLIC_FUNCTION LavError Lav_createRecorderNode(LavHandle simulationHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_recorderNodeStartRecording(LavHandle nodeHandle, const char* path);
Lav_PUBLIC_FUNCTION LavError Lav_recorderNodeStopRecording(LavHandle nodeHandle);

Lav_PUBLIC_FUNCTION LavError Lav_createConvolverNode(LavHandle simulationHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createFftConvolverNode(LavHandle simulationHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_fftConvolverNodeSetResponse(LavHandle nodeHandle, int channel, int length, float* response);
Lav_PUBLIC_FUNCTION LavError Lav_fftConvolverNodeSetResponseFromFile(LavHandle nodeHandle, const char* path, int fileChannel, int convolverChannel);

Lav_PUBLIC_FUNCTION LavError Lav_createThreeBandEqNode(LavHandle simulationHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createFilteredDelayNode(LavHandle simulationHandle, float maxDelay, unsigned int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createPannerBankNode(LavHandle simulationHandle, int pannerCount, char* hrtfPath, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createCrossfaderNode(LavHandle simulationHandle, int channels, int inputs, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_crossfaderNodeCrossfade(LavHandle nodeHandle, float duration, int input);
Lav_PUBLIC_FUNCTION LavError Lav_crossfaderNodeSetFinishedCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);

Lav_PUBLIC_FUNCTION LavError Lav_createOnePoleFilterNode(LavHandle simulationHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createFirstOrderFilterNode(LavHandle simulationHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_firstOrderFilterNodeConfigureLowpass(LavHandle nodeHandle, float frequency);
Lav_PUBLIC_FUNCTION LavError Lav_firstOrderFilterNodeConfigureHighpass(LavHandle nodeHandle, float frequency);
Lav_PUBLIC_FUNCTION LavError Lav_firstOrderFilterNodeConfigureAllpass(LavHandle nodeHandle, float frequency);

Lav_PUBLIC_FUNCTION LavError Lav_createAllpassNode(LavHandle simulationHandle, int channels, int maxDelay, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createNestedAllpassNetworkNode(LavHandle simulationHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeBeginNesting(LavHandle nodeHandle, int delay, float coefficient);
Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeEndNesting(LavHandle nodeHandle);
Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeAppendAllpass(LavHandle nodeHandle, int delay, float coefficient);
Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeAppendOnePole(LavHandle nodeHandle, float frequency, int isHighpass);
Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeAppendBiquad(LavHandle nodeHandle, int type, double frequency, double dbGain, double q);
Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeAppendReader(LavHandle nodeHandle, float mul);
Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeCompile(LavHandle nodeHandle);

Lav_PUBLIC_FUNCTION LavError Lav_createFdnReverbNode(LavHandle simulationHandle, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createBlitNode(LavHandle simulationHandle, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createDcBlockerNode(LavHandle simulationHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createLeakyIntegratorNode(LavHandle simulationHandle, int channels, LavHandle* destination);

#ifdef __cplusplus
}
#endif