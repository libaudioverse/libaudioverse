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
	Lav_ERROR_INVALID_SLOT, //one of the functions taking a slot got passed an invalid number.
	Lav_ERROR_NULL_POINTER, //you passed a NULL pointer into something that shouldn't have it.
	Lav_ERROR_MEMORY, //a memory problem which probably isn't the fault of the application.
	Lav_ERROR_INVALID_HANDLE,
	Lav_ERROR_RANGE, //out of range function parameter.
	Lav_ERROR_CANNOT_INIT_AUDIO, //We couldn't initialize the audio library or open a device
	Lav_ERROR_FILE, //error to do with files.
	Lav_ERROR_FILE_NOT_FOUND, //specifically, we couldn't find a file.

	Lav_ERROR_HRTF_INVALID,

	/**This one is odd.  It is what is thrown if you pass a node with the wrong "shape" to a function, most notably source creation.*/
	Lav_ERROR_SHAPE,

	Lav_ERROR_CANNOT_CROSS_DEVICES, //an attempto either create a parent-child connect with nodes from different devices or to set an output with an node from a different device.
	Lav_ERROR_NO_OUTPUTS, //we expected the node to have outputs here, but it didn't.
	Lav_ERROR_LIMIT_EXCEEDED,
	Lav_ERROR_CAUSES_CYCLE,
	Lav_ERROR_PROPERTY_IS_READ_ONLY,

	Lav_ERROR_OVERLAPPING_AUTOMATORS,

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
	Lav_OBJTYPE_SIMPLE_ENVIRONMENT_NODE,
	Lav_OBJTYPE_SOURCE_NODE,
	Lav_OBJTYPE_FILE_NODE,
	Lav_OBJTYPE_HRTF_NODE,
	Lav_OBJTYPE_SINE_NODE,
	Lav_OBJTYPE_HARD_LIMITER_NODE,
	Lav_OBJTYPE_DELAY_NODE,
	Lav_OBJTYPE_AMPLITUDE_PANNER_NODE,
	Lav_OBJTYPE_PUSH_NODE,
	Lav_OBJTYPE_BIQUAD_NODE,
	Lav_OBJTYPE_PULL_NODE,
	Lav_OBJTYPE_GRAPH_LISTENER_NODE,
	Lav_OBJTYPE_CUSTOM_NODE,
	Lav_OBJTYPE_RINGMOD_NODE,
	Lav_OBJTYPE_MULTIPANNER_NODE,
	Lav_OBJTYPE_FEEDBACK_DELAY_NETWORK_NODE,
	Lav_OBJTYPE_MULTIFILE_NODE,
	Lav_OBJTYPE_SQUARE_NODE,
	Lav_OBJTYPE_NOISE_NODE,
	Lav_OBJTYPE_IIR_NODE,
	Lav_OBJTYPE_GAIN_NODE,
	Lav_OBJTYPE_CHANNEL_SPLITTER_NODE,
	Lav_OBJTYPE_CHANNEL_MERGER_NODE,
	Lav_OBJTYPE_BUFFER_NODE,
};

/**Node states.*/
enum Lav_NODE_STATES {
	Lav_NODESTATE_PAUSED,
	Lav_NODESTATE_PLAYING,
	Lav_NODESTATE_ALWAYS_PLAYING,
};

/**Logging levels.*/
enum Lav_LOGGING_LEVELS {
	Lav_LOG_LEVEL_OFF = 0,
	Lav_LOG_LEVEL_CRITICAL = 1,
	Lav_LOG_LEVEL_INFO = 2,
	Lav_LOG_LEVEL_DEBUG = 3,
};

/**Initialize Libaudioverse.*/
Lav_PUBLIC_FUNCTION LavError Lav_initialize();
/**Shuts down the library.

After a call to this function, any Libaudioverse handles are invalid.
So too are any pointers that Libaudioverse gave you, of any form.*/
Lav_PUBLIC_FUNCTION LavError Lav_shutdown();
Lav_PUBLIC_FUNCTION LavError Lav_isInitialized(int* destination);

/**Free any pointer that libaudioverse gives you.  If something goes wrong, namely that the pointer isn't from Libaudioverse in the first place, this tries to fail gracefully and give you an error, but don't rely on this.*/
Lav_PUBLIC_FUNCTION LavError Lav_free(void* obj);

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
typedef void (*LavLoggingCallback)(int level, const char* message, int is_final);
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
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetLatency(unsigned int index, float* destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetName(unsigned int index, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetChannels(unsigned int index, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createSimulationForDevice(int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead, LavHandle* destination);

/**This type of simulation is intended for apps that wish to handle audio themselves: it will not output and time will not advance for it.
Combine it with Lav_simulationReadBlock to make use of it.*/
Lav_PUBLIC_FUNCTION LavError Lav_createReadSimulation(unsigned int sr, unsigned int blockSize, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlockSize(LavHandle simulationHandle, int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlock(LavHandle simulationHandle, unsigned int channels, int mayApplyMixingMatrix, float* buffer);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetSr(LavHandle simulationHandle, int* destination);

/**Atomic block support.
Todo: find a better name.
This isn't truly atomic: operations you perform will not roll back on error.
When an atomic block is begun, the net effect is that the current thread holds a lock to the libaudioverse simulation. If your code blocks, Libaudioverse cannot mix.
Every call to Lav_simulationBeginAtomicBlock must be matched with a call to Lav_simulationEndAtomicBlock or audio will stop. Atomic blocks do nest.
*/
Lav_PUBLIC_FUNCTION LavError Lav_simulationBeginAtomicBlock(LavHandle simulationHandle);
Lav_PUBLIC_FUNCTION LavError Lav_simulationEndAtomicBlock(LavHandle simulationHandle);

/**The block callback.
This can be used in some situations for precise timing.
This callback is called with a handle to the simulation and a double.  The double is the time in seconds since the beginning of the block at which this callback was first called.
For audio output devices, this callback is called in the mixing thread.  If it blocks, audio stops. Don't.
It is safe to call Libaudioverse from this callback.
To clear, use null as the callback.*/
typedef void (*LavBlockCallback)(LavHandle handle, double time, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_simulationSetBlockCallback(LavHandle simulation, LavBlockCallback callback, void* userdata);

/**Buffers.
Buffers are chunks of audio data from any source.  A variety of nodes to work with buffers exist.*/
Lav_PUBLIC_FUNCTION LavError Lav_createBuffer(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferGetSimulation(LavHandle handle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromFile(LavHandle bufferHandle, const char* path);
Lav_PUBLIC_FUNCTION LavError Lav_bufferLoadFromArray(LavHandle bufferHandle, int sr, int channels, int frames, float* data);

Lav_PUBLIC_FUNCTION LavError Lav_nodeGetSimulation(LavHandle handle, LavHandle* destination);
/**Connect two nodes.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeConnect(LavHandle nodeHandle, int output, LavHandle destHandle, int input);
/**Connect the specified output to the simulation.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeConnectSimulation(LavHandle nodeHandle, int output);
/**Kill all connections involving the output specified.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeDisconnect(LavHandle nodeHandle, int output);

/**Query node type.*/

/**Query maximum number of inputs and outputs.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetInputConnectionCount(LavHandle nodeHandle, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetOutputConnectionCount(LavHandle nodeHandle, unsigned int* destination);

/**Resets a property to its default value, for any type.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeResetProperty(LavHandle nodeHandle, int slot);

/**Property getters and setters.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetIntProperty(LavHandle nodeHandle, int slot, int value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloatProperty(LavHandle nodeHandle, int slot, float value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetDoubleProperty(LavHandle nodeHandle, int slot, double value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetStringProperty(LavHandle nodeHandle, int slot, char* value);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat3Property(LavHandle nodeHandle, int slot, float v1, float v2, float v3);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetFloat6Property(LavHandle nodeHandle, int slot, float v1, float v2, float v3, float v4, float v5, float v6);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntProperty(LavHandle nodeHandle, int slot, int *destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatProperty(LavHandle nodeHandle, int slot, float *destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoubleProperty(LavHandle nodeHandle, int slot, double *destination);
//Note: allocates memory for you, that should be freed with Lav_free.  This is a convenience for those using higher level languages, and to avoid having to make second queries for length.
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetStringProperty(LavHandle nodeHandle, int slot, const char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat3Property(LavHandle nodeHandle, int slot, float* destination1, float* destination2, float* destination3);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloat6Property(LavHandle nodeHandle, int slot, float* destinationV1, float* destinationV2, float* destinationV3, float* destinationV4, float* destinationV5, float* destinationV6);

/**Query property ranges. These are set only by internal code.  Float3 and Float6 are effectively rangeless: specifically what a range is for those is very undefined and specific to the node in question.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntPropertyRange(LavHandle nodeHandle, int slot, int* destination_lower, int* destination_upper);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatPropertyRange(LavHandle nodeHandle, int slot, float* destination_lower, float* destination_upper);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetDoublePropertyRange(LavHandle nodeHandle, int slot, double* destination_lower, double* destination_upper);


Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyName(LavHandle nodeHandle, int slot, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyType(LavHandle nodeHandle, int slot, int* destination);
/**Properties with dynamic ranges may change the endpoints of their range at any time and for any reason.
This is primarily used by bindings, but may be useful to user interface developers.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetPropertyHasDynamicRange(LavHandle nodeHandle, int slot, int* destination);

/**Array properties.
An array property does not have a range. Instead, it has a minimum and maximum supported length.
Note that these provide functions for setting parts of the array: the property can store any size of 1-dimensional array.

In order to make reading sane, you can only read one value at a time. Since Libaudioverse never exposes internal memory to the public user, reading multiple values would require wasting huge amounts of ram.
*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceFloatArrayProperty(LavHandle nodeHandle, int slot, unsigned int length, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReadFloatArrayProperty(LavHandle nodeHandle, int slot, unsigned int index, float* destination);
Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteFloatArrayProperty(LavHandle nodeHandle, int slot, unsigned int start, unsigned int stop, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatArrayPropertyDefault(LavHandle nodeHandle, int slot, unsigned int* destinationLength, float** destinationArray);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetFloatArrayPropertyLength(LavHandle nodeHandle, int slot, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReplaceIntArrayProperty(LavHandle nodeHandle, int slot, unsigned int length, int* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeReadIntArrayProperty(LavHandle nodeHandle, int slot, unsigned int index, int* destination);
Lav_PUBLIC_FUNCTION LavError  Lav_nodeWriteIntArrayProperty(LavHandle nodeHandle, int slot, unsigned int start, unsigned int stop, int* values);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntArrayPropertyDefault(LavHandle nodeHandle, int slot, unsigned int* destinationLength, int** destinationArray);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetIntArrayPropertyLength(LavHandle nodeHandle, int slot, int* destination);
//applies to eather array type.
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetArrayPropertyLengthRange(LavHandle nodeHandle, int slot, unsigned int* destinationMin, unsigned int* destinationMax);

//buffer properties.
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetBufferProperty(LavHandle nodeHandle, int slot, LavHandle bufferHandle);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetBufferProperty(LavHandle nodeHandle, int slot, LavHandle* destination);

/**Automators.
These apply only to float and double properties. Times are relative to "now" in node time, not simulation time.
If setting up complex timelines, do it inside an atomic block.
*/
//Linear ramp to value starting after the last event and ending at time t.
Lav_PUBLIC_FUNCTION LavError Lav_automationLinearRampToValue(LavHandle nodeHandle, int slot, double time, double value);

/**events.
Unlike callbacks, which are dedicated on a per-node basis, events fire in a background thread. It is safe to call the Libaudioverse API from a firing event.
It is documented which of these will fire more than once.  Most events will wait for you to return from the handler before firing a duplicate event.  It is wise to be as quick as possible.*/
EXTERN_FUNCTION typedef void (*LavEventCallback)(LavHandle cause, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetEventHandler(LavHandle node, int event, LavEventCallback *destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeGetEventUserDataPointer(LavHandle node, int event, void** destination);
Lav_PUBLIC_FUNCTION LavError Lav_nodeSetEvent(LavHandle node, int event, LavEventCallback handler, void* userData);

/**Performs the node-specific reset operation.

This does not reset properties, merely internal histories and the like.  Specifically what this means depends on the node; see the manual.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeReset(LavHandle nodeHandle);

//creators for different objject types.
//also see libaudioverse3d.h.

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createSquareNode(LavHandle simulationHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createNoiseNode(LavHandle simulationHandle, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createFileNode(LavHandle simulationHandle, const char* path, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createHrtfNode(LavHandle simulationHandle, const char* hrtfPath, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterNode(LavHandle simulationHandle, unsigned int numInputs, LavHandle *destination);
Lav_PUBLIC_FUNCTION LavError Lav_createDelayNode(LavHandle simulationHandle, float maxDelay, unsigned int lineCount, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerNode(LavHandle simulationHandle, LavHandle* destination);
//can set the standard channel map for 2, 6, and 8 channels. Other values cause Lav_ERROR_RANGE.
Lav_PUBLIC_FUNCTION LavError Lav_amplitudePannerNodeConfigureStandardMap(LavHandle nodeHandle, unsigned int channels);

Lav_PUBLIC_FUNCTION LavError Lav_createMultipannerNode(LavHandle simulationHandle, char* hrtfPath, LavHandle* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createPushNode(LavHandle simulationHandle, unsigned int sr, unsigned int channels, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_pushNodeFeed(LavHandle nodeHandle, unsigned int length, float* buffer);
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
Lav_PUBLIC_FUNCTION LavError Lav_createFeedbackDelayNetworkNode(LavHandle simulationHandle, float maxDelay, int lines, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkNodeSetFeedbackMatrix(LavHandle node, int count, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkNodeSetOutputGains(LavHandle nodeHandle, int count, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkNodeSetDelays(LavHandle nodeHandle , int count, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkNodeSetFeedbackDelayMatrix(LavHandle nodeHandle, int count, float* values);

Lav_PUBLIC_FUNCTION LavError Lav_createMultifileNode(LavHandle simulationHandle, int channels, int maxSimultaneousFiles, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_multifileNodePlay(LavHandle nodeHandle, char* path);
Lav_PUBLIC_FUNCTION LavError Lav_multifileNodeStopAll(LavHandle nodeHandle);

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

#ifdef __cplusplus
}
#endif