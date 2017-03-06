/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/**The public interface to Libaudioverse.*/

/*Forward-declares all Libaudioverse types.*/

typedef int LavError;
typedef int LavHandle;
typedef void (*LavParameterlessCallback)(LavHandle nodeHandle, void* userdata);
//Like parameterless callback, but for callbacks that need to know about time.
typedef void (*LavTimeCallback)(LavHandle handle, double time, void* userdata);

/**Does whatever is appropriate on a given platform to expose a Libaudioverse function publically.*/
#ifdef _MSC_VER
#ifdef LIBAUDIOVERSE_IS_LIBRARY
#define DLL_PUBLIC_ATTR __declspec(dllexport)
#else
#define DLL_PUBLIC_ATTR __declspec(dllimport)
#endif
#endif
#ifdef __GNUC__
#define DLL_PUBLIC_ATTR __attribute__ ((visibility ("default")))
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
	Lav_ERROR_NOT_INITIALIZED,
	Lav_ERROR_TYPE_MISMATCH, //Tried to get/set something with the wrong type, i.e. properties.
	Lav_ERROR_INVALID_PROPERTY, //one of the functions taking a slot got passed an invalid number.
	Lav_ERROR_NULL_POINTER, //you passed a NULL pointer into something that shouldn't have it.
	Lav_ERROR_MEMORY, //a memory problem which probably isn't the fault of the application.
	Lav_ERROR_INVALID_POINTER,
	Lav_ERROR_INVALID_HANDLE,
	Lav_ERROR_RANGE, //out of range function parameter.
	Lav_ERROR_CANNOT_INIT_AUDIO, //We couldn't initialize the audio library or open a device
	Lav_ERROR_NO_SUCH_DEVICE,
	Lav_ERROR_FILE, //error to do with files.
	Lav_ERROR_FILE_NOT_FOUND, //specifically, we couldn't find a file.

	Lav_ERROR_HRTF_INVALID,

	Lav_ERROR_CANNOT_CROSS_SERVERS, 

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
	Lav_OBJTYPE_SERVER,
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
	Lav_OBJTYPE_CROSSFADER_NODE,
	Lav_OBJTYPE_ONE_POLE_FILTER_NODE,
	Lav_OBJTYPE_FIRST_ORDER_FILTER_NODE,
	Lav_OBJTYPE_ALLPASS_NODE,
	Lav_OBJTYPE_FDN_REVERB_NODE,
	Lav_OBJTYPE_BLIT_NODE,
	Lav_OBJTYPE_DC_BLOCKER_NODE,
	Lav_OBJTYPE_LEAKY_INTEGRATOR_NODE,
	Lav_OBJTYPE_FILE_STREAMER_NODE,
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

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetCount(unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetName(unsigned int index, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetIdentifierString(unsigned int index, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetChannels(unsigned int index, unsigned int* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createServer(unsigned int sr, unsigned int blockSize, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_serverGetBlockSize(LavHandle serverHandle, int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_serverGetBlock(LavHandle serverHandle, unsigned int channels, int mayApplyMixingMatrix, float* buffer);
Lav_PUBLIC_FUNCTION LavError Lav_serverGetSr(LavHandle serverHandle, int* destination);

/**Set or clear the output device.*/
Lav_PUBLIC_FUNCTION LavError Lav_serverSetOutputDevice(LavHandle serverHandle, const char* device, int channels, int mixahead);
Lav_PUBLIC_FUNCTION LavError Lav_serverClearOutputDevice(LavHandle serverHandle);

Lav_PUBLIC_FUNCTION LavError Lav_serverLock(LavHandle serverHandle);
Lav_PUBLIC_FUNCTION LavError Lav_serverUnlock(LavHandle serverHandle);

Lav_PUBLIC_FUNCTION LavError Lav_serverSetBlockCallback(LavHandle serverHandle, LavTimeCallback callback, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_serverWriteFile(LavHandle serverHandle, const char* path, int channels, double duration, int mayApplyMixingMatrix);

Lav_PUBLIC_FUNCTION LavError Lav_serverSetThreads(LavHandle serverHandle, int threads);
Lav_PUBLIC_FUNCTION LavError Lav_serverGetThreads(LavHandle serverHandle, int* destination);

Lav_PUBLIC_FUNCTION LavError Lav_serverCallIn(LavHandle serverHandle, double when, int inAudioThread, LavTimeCallback cb, void* userdata);

/**Buffers.
Buffers are chunks of audio data from any source.  A variety of nodes to work with buffers exist.*/
Lav_PUBLIC_FUNCTION LavError Lav_createBuffer(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferGetServer(LavHandle bufferHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromFile(LavHandle bufferHandle, const char* path);
Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromArray(LavHandle bufferHandle, int sr, int channels, int frames, float* data);
Lav_PUBLIC_FUNCTION LavError Lav_bufferDecodeFromArray(LavHandle bufferHandle, char* data, int datalen);
Lav_PUBLIC_FUNCTION LavError Lav_bufferNormalize(LavHandle bufferHandle);
Lav_PUBLIC_FUNCTION LavError Lav_bufferGetDuration(LavHandle bufferHandle, float* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferGetLengthInSamples(LavHandle bufferHandle, int* destination);

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetServer(LavHandle nodeHandle, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_nodeConnect(LavHandle nodeHandle, int output, LavHandle destHandle, int input);
Lav_PUBLIC_FUNCTION LavError Lav_nodeConnectServer(LavHandle nodeHandle, int output);
Lav_PUBLIC_FUNCTION LavError Lav_nodeConnectProperty(LavHandle nodeHandle, int output, LavHandle otherHandle, int slot);
Lav_PUBLIC_FUNCTION LavError Lav_nodeDisconnect(LavHandle nodeHandle, int output, LavHandle otherHandle, int input);
Lav_PUBLIC_FUNCTION LavError Lav_nodeIsolate(LavHandle nodeHandle);

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputConnectionCount(LavHandle nodeHandle, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetOutputConnectionCount(LavHandle nodeHandle, unsigned int* destination);


Lav_PUBLIC_FUNCTION LavError Lav_nodeResetProperty(LavHandle nodeHandle, int propertyIndex);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetIntProperty(LavHandle nodeHandle, int propertyIndex, int value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloatProperty(LavHandle nodeHandle, int propertyIndex, float value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetDoubleProperty(LavHandle nodeHandle, int propertyIndex, double value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetStringProperty(LavHandle nodeHandle, int propertyIndex, char* value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat3Property(LavHandle nodeHandle, int propertyIndex, float v1, float v2, float v3);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat6Property(LavHandle nodeHandle, int propertyIndex, float v1, float v2, float v3, float v4, float v5, float v6);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntProperty(LavHandle nodeHandle, int propertyIndex, int *destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatProperty(LavHandle nodeHandle, int propertyIndex, float *destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoubleProperty(LavHandle nodeHandle, int propertyIndex, double *destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetStringProperty(LavHandle nodeHandle, int propertyIndex, const char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat3Property(LavHandle nodeHandle, int propertyIndex, float* destination1, float* destination2, float* destination3);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat6Property(LavHandle nodeHandle, int propertyIndex, float* destinationV1, float* destinationV2, float* destinationV3, float* destinationV4, float* destinationV5, float* destinationV6);

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntPropertyRange(LavHandle nodeHandle, int propertyIndex, int* destinationMin, int* destinationMax);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatPropertyRange(LavHandle nodeHandle, int propertyIndex, float* destinationMin, float* destinationMax);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoublePropertyRange(LavHandle nodeHandle, int propertyIndex, double* destinationMin, double* destinationMax);

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyName(LavHandle nodeHandle, int propertyIndex, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyType(LavHandle nodeHandle, int propertyIndex, int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyHasDynamicRange(LavHandle nodeHandle, int propertyIndex, int* destination);

Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceFloatArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int length, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReadFloatArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int index, float* destination);
Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteFloatArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int start, unsigned int stop, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatArrayPropertyLength(LavHandle nodeHandle, int propertyIndex, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceIntArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int length, int* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReadIntArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int index, int* destination);
Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteIntArrayProperty(LavHandle nodeHandle, int propertyIndex, unsigned int start, unsigned int stop, int* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntArrayPropertyLength(LavHandle nodeHandle, int propertyIndex, int* destination);

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetArrayPropertyLengthRange(LavHandle nodeHandle, int propertyIndex, unsigned int* destinationMin, unsigned int* destinationMax);

Lav_PUBLIC_FUNCTION LavError Lav_nodeSetBufferProperty(LavHandle nodeHandle, int propertyIndex, LavHandle value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetBufferProperty(LavHandle nodeHandle, int propertyIndex, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_automationCancelAutomators(LavHandle nodeHandle, int propertyIndex, double time);
Lav_PUBLIC_FUNCTION LavError Lav_automationLinearRampToValue(LavHandle nodeHandle, int slot, double time, double value);
Lav_PUBLIC_FUNCTION LavError Lav_automationSet(LavHandle nodeHandle, int slot, double time, double value);
Lav_PUBLIC_FUNCTION LavError Lav_automationEnvelope(LavHandle nodeHandle, int slot, double time, double duration, int valuesLength, double *values);

Lav_PUBLIC_FUNCTION LavError Lav_nodeReset(LavHandle nodeHandle);

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createAdditiveSquareNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createAdditiveTriangleNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createAdditiveSawNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createNoiseNode(LavHandle serverHandle, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfNode(LavHandle serverHandle, const char* hrtfPath, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterNode(LavHandle serverHandle, int channels, LavHandle *destination);

Lav_PUBLIC_FUNCTION LavError Lav_createCrossfadingDelayNode(LavHandle serverHandle, float maxDelay, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createDoppleringDelayNode(LavHandle serverHandle, float maxDelay, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_amplitudePannerNodeConfigureStandardMap(LavHandle nodeHandle, unsigned int channels);

Lav_PUBLIC_FUNCTION LavError Lav_createMultipannerNode(LavHandle serverHandle, char* hrtfPath, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createPushNode(LavHandle serverHandle, unsigned int sr, unsigned int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_pushNodeFeed(LavHandle nodeHandle, unsigned int length, float* frames);
Lav_PUBLIC_FUNCTION LavError Lav_pushNodeSetLowCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_pushNodeSetUnderrunCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);


Lav_PUBLIC_FUNCTION LavError Lav_createBiquadNode(LavHandle serverHandle, unsigned int channels, LavHandle* destination);


typedef void (*LavPullNodeAudioCallback)(LavHandle nodeHandle, int frames, int channels, float* buffer, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_createPullNode(LavHandle serverHandle, unsigned int sr, unsigned int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_pullNodeSetAudioCallback(LavHandle nodeHandle, LavPullNodeAudioCallback callback, void* userdata);

typedef void (*LavGraphListenerNodeListeningCallback)(LavHandle nodeHandle, unsigned int frames, unsigned int channels, float* buffer, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_createGraphListenerNode(LavHandle serverHandle, unsigned int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_graphListenerNodeSetListeningCallback(LavHandle nodeHandle, LavGraphListenerNodeListeningCallback callback, void* userdata);

Lav_PUBLIC_FUNCTION LavError Lav_createRingmodNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createFeedbackDelayNetworkNode(LavHandle serverHandle, float maxDelay, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createIirNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_iirNodeSetCoefficients(LavHandle nodeHandle, int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory);

Lav_PUBLIC_FUNCTION LavError Lav_createGainNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createChannelSplitterNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createChannelMergerNode(LavHandle serverHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createBufferNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferNodeSetEndCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);

Lav_PUBLIC_FUNCTION LavError Lav_createBufferTimelineNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferTimelineNodeScheduleBuffer(LavHandle nodeHandle, LavHandle bufferHandle, double time, float pitchBend);

Lav_PUBLIC_FUNCTION LavError Lav_createRecorderNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_recorderNodeStartRecording(LavHandle nodeHandle, const char* path);
Lav_PUBLIC_FUNCTION LavError Lav_recorderNodeStopRecording(LavHandle nodeHandle);

Lav_PUBLIC_FUNCTION LavError Lav_createConvolverNode(LavHandle serverHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createFftConvolverNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_fftConvolverNodeSetResponse(LavHandle nodeHandle, int channel, int length, float* response);
Lav_PUBLIC_FUNCTION LavError Lav_fftConvolverNodeSetResponseFromFile(LavHandle nodeHandle, const char* path, int fileChannel, int convolverChannel);

Lav_PUBLIC_FUNCTION LavError Lav_createThreeBandEqNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createFilteredDelayNode(LavHandle serverHandle, float maxDelay, unsigned int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createCrossfaderNode(LavHandle serverHandle, int channels, int inputs, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_crossfaderNodeCrossfade(LavHandle nodeHandle, float duration, int input);
Lav_PUBLIC_FUNCTION LavError Lav_crossfaderNodeSetFinishedCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);

Lav_PUBLIC_FUNCTION LavError Lav_createOnePoleFilterNode(LavHandle serverHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createFirstOrderFilterNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_firstOrderFilterNodeConfigureLowpass(LavHandle nodeHandle, float frequency);
Lav_PUBLIC_FUNCTION LavError Lav_firstOrderFilterNodeConfigureHighpass(LavHandle nodeHandle, float frequency);
Lav_PUBLIC_FUNCTION LavError Lav_firstOrderFilterNodeConfigureAllpass(LavHandle nodeHandle, float frequency);

Lav_PUBLIC_FUNCTION LavError Lav_createAllpassNode(LavHandle serverHandle, int channels, int maxDelay, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createFdnReverbNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createBlitNode(LavHandle serverHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createDcBlockerNode(LavHandle serverHandle, int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createLeakyIntegratorNode(LavHandle serverHandle, int channels, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createFileStreamerNode(LavHandle serverHandle, const char* path, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_fileStreamerNodeSetEndCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata);

#ifdef __cplusplus
}
#endif