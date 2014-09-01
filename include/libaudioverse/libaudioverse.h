/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/**The public interface to Libaudioverse.*/

/*Forward-declares all Libaudioverse types.*/
#ifdef __cplusplus
class LavObject;
class LavSimulation;
#else
typedef void LavSimulation;
typedef void LavObject;
#endif
/**Make sure that we are marshalling enums as integers in the public API for both c and c++.
Note that, internally, it's safe to use enums directly.  But marshalling out to some languages with the enum type is potentially dangerous: the standard does not designate what an enum is internally represented as, so we rely on the implicit conversion.*/
typedef int LavError;
typedef int LavLimits;
typedef int LavPropertyType;
typedef int LavObjectType;

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

enum Lav_LIMITS {
	Lav_MAX_BLOCK_SIZE = 1024,
};

enum Lav_ERRORS {
	Lav_ERROR_NONE = 0, //everything is OK.
	Lav_ERROR_UNKNOWN = 1, //We know something has gone wrong, but can't figure out what.
	Lav_ERROR_TYPE_MISMATCH = 2, //Tried to get/set something with the wrong type, i.e. properties.
	Lav_ERROR_INVALID_SLOT = 3, //one of the functions taking a slot got passed an invalid number.
	Lav_ERROR_NULL_POINTER = 4, //you passed a NULL pointer into something that shouldn't have it.
	Lav_ERROR_MEMORY = 5, //a memory problem which probably isn't the fault of the application.
	Lav_ERROR_RANGE = 6, //out of range function parameter.
	Lav_ERROR_CANNOT_INIT_AUDIO = 7, //We couldn't even initialize the audio library.
	Lav_ERROR_FILE = 8, //error to do with files.
	Lav_ERROR_FILE_NOT_FOUND = 9, //specifically, we couldn't find a file.

	Lav_ERROR_HRTF_INVALID = 10,

	/**This one is odd.  It is what is thrown if you pasas a object with the wrong "shape" to a function, most notably source creation.*/
	Lav_ERROR_SHAPE = 11,

	Lav_ERROR_CANNOT_CROSS_DEVICES = 12, //an attempto either create a parent-child connect with objects from different devices or to set an output with an object from a different device.
	Lav_ERROR_NO_OUTPUTS = 13, //we expected the object to have outputs here, but it didn't.
	Lav_ERROR_LIMIT_EXCEEDED = 14,
	Lav_ERROR_CAUSES_CYCLE = 15,
	Lav_ERROR_INTERNAL= 999,
};

/**These are property types, either int, float, double, vector of 3 floats, vector of 6 floats, string, or int/float array of any length.*/
enum Lav_PROPERTYTYPES {
	Lav_PROPERTYTYPE_INT = 1,
	Lav_PROPERTYTYPE_FLOAT = 2,
	Lav_PROPERTYTYPE_DOUBLE = 3,
	Lav_PROPERTYTYPE_STRING = 4,
	Lav_PROPERTYTYPE_FLOAT3 = 5,
	Lav_PROPERTYTYPE_FLOAT6 = 6,
	Lav_PROPERTYTYPE_FLOAT_ARRAY = 7,
	Lav_PROPERTYTYPE_INT_ARRAY = 8,
};

/**These are used to tag objects with their type, so that external languages may see them.*/
enum Lav_OBJTYPES {
	Lav_OBJTYPE_GENERIC = 0, //this is not something you should ever see outside the library, and basically means none.
	Lav_OBJTYPE_WORLD = 1,
	Lav_OBJTYPE_SOURCE = 2,
	Lav_OBJTYPE_FILE = 3,
	Lav_OBJTYPE_HRTF = 4,
	Lav_OBJTYPE_SINE = 5,
	Lav_OBJTYPE_MIXER = 6,
	Lav_OBJTYPE_ATTENUATOR = 7,
	Lav_OBJTYPE_HARD_LIMITER = 8,
	Lav_OBJTYPE_DELAY = 9,
	Lav_OBJTYPE_AMPLITUDE_PANNER = 10,
};

/**Object states.*/
enum Lav_OBJECT_STATES {
	Lav_OBJECT_STATE_SUSPENDED = 0,
	Lav_OBJECT_STATE_PAUSED = 1,
	Lav_OBJECT_STATE_PLAYING = 2,
	Lav_OBJECT_STATE_ALWAYS_PLAYING = 3
};

/**Initialize Libaudioverse.*/
Lav_PUBLIC_FUNCTION LavError Lav_initializeLibrary();

/**Free any pointer that libaudioverse gives you.  If something goes wrong, namely that the pointer isn't from Libaudioverse in the first place, this tries to fail gracefully, but usually can't.*/
Lav_PUBLIC_FUNCTION LavError Lav_free(void* obj);

//devices...
/**
Index -1 is special.  Any attempts to query about index -1 will fail.  Index -1 is always 2 channels.
Opening a device on index -1 requests that the default system audio device be used.  In addition, however, index -1 will follow the default system audio device when the backend supports it.
It is constrained to two channels because there is no graceful way to handle moving to a device with less channels, and it is assumed apps looking for this functionality will not mind the loss.  This may/will probably be changed in future.
*/
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetCount(unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetLatency(unsigned int index, float* destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetName(unsigned int index, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetChannels(unsigned int index, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_createSimulationForDevice(int index, unsigned int sr, unsigned int blockSize, unsigned int mixAhead, LavSimulation** destination);

/**This type of simulation is intended for apps that wish to handle audio themselves: it will not output and time will not advance for it.
Combine it with Lav_simulationReadBlock to make use of it.*/
Lav_PUBLIC_FUNCTION LavError Lav_createReadSimulation(unsigned int sr, unsigned int channels, unsigned int blockSize, LavSimulation** destination);

Lav_PUBLIC_FUNCTION LavError Lav_simulationSetOutputObject(LavSimulation* simulation, LavObject* object);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetOutputObject(LavSimulation* simulation, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlockSize(LavSimulation* simulation, int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlock(LavSimulation* simulation, float* buffer);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetSr(LavSimulation* simulation, int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_simulationGetChannels(LavSimulation* simulation, int* destination);

/**Query object type.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetType(LavObject* obj, int* destination);

/**Query maximum number of inputs and outputs.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputCount(LavObject* obj, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetOutputCount(LavObject* obj, unsigned int* destination);

/**Input management.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputObject(LavObject *obj, unsigned int slot, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputOutput(LavObject* obj, unsigned int slot, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetInput(LavObject *obj, unsigned int input, LavObject* parent, unsigned int output);

/**Resets a property to its default value, for any type.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectResetProperty(LavObject *obj, int slot);

/**Property getters and setters.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectSetIntProperty(LavObject* obj, int slot, int value);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloatProperty(LavObject *obj, int slot, float value);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetDoubleProperty(LavObject *obj, int slot, double value);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetStringProperty(LavObject*obj, int slot, char* value);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat3Property(LavObject* obj, int slot, float v1, float v2, float v3);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat6Property(LavObject* obj, int slot, float v1, float v2, float v3, float v4, float v5, float v6);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntProperty(LavObject*obj, int slot, int *destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatProperty(LavObject* obj, int slot, float *destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoubleProperty(LavObject*obj, int slot, double *destination);
//Note: allocates memory for you, that should be freed with Lav_free.  This is a convenience for those using higher level languages, and to avoid having to make second queries for length.
Lav_PUBLIC_FUNCTION LavError Lav_objectGetStringProperty(LavObject* obj, int slot, const char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat3Property(LavObject* obj, int slot, float* v1, float* v2, float* v3);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat6Property(LavObject* obj, int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6);

/**Query property ranges. These are set only by internal code.  Float3 and Float6 are effectively rangeless: specifically what a range is for those is very undefined and specific to the object in question.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntPropertyRange(LavObject* obj, int slot, int* destination_lower, int* destination_upper);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatPropertyRange(LavObject* obj, int slot, float* destination_lower, float* destination_upper);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoublePropertyRange(LavObject* obj, int slot, double* destination_lower, double* destination_upper);

/**Get the indices of all nondynamic properties on an object as an array of ints, that is all properties with numeric index less than 0.  Allocates memory; free with Lav_free.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetPropertyIndices(LavObject* obj, int** destination);
/**Get the name of a property.  Again, allocates memory.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetPropertyName(LavObject* obj, int slot, char** destination);

/**Array properties.
An array property does not have a range. Instead, it has a minimum and maximum supported length.
Note that these provide functions for setting parts of the array: the property can store any size of 1-dimensional array.

In order to make reading sane, you can only read one value at a time. Since Libaudioverse never exposes internal memory to the public user, reading multiple values would require wasting huge amounts of ram.
*/
Lav_PUBLIC_FUNCTION LavError Lav_objectReplaceFloatArrayProperty(LavObject* obj, int slot, unsigned int length, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_objectReadFloatArrayProperty(LavObject* obj, int slot, unsigned int index, float* destination);
Lav_PUBLIC_FUNCTION LavError  Lav_objectWriteFloatArrayProperty(LavObject* obj, int slot, unsigned int start, unsigned int stop, float* values);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatArrayPropertyDefault(LavObject* obj, int slot, unsigned int* destinationLength, float** destinationArray);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatArrayPropertyLength(LavObject* obj, int slot, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectReplaceIntArrayProperty(LavObject* obj, int slot, unsigned int length, int* values);
Lav_PUBLIC_FUNCTION LavError Lav_objectReadIntArrayProperty(LavObject* obj, int slot, unsigned int index, int* destination);
Lav_PUBLIC_FUNCTION LavError  Lav_objectWriteIntArrayProperty(LavObject* obj, int slot, unsigned int start, unsigned int stop, int* values);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntArrayPropertyDefault(LavObject* obj, int slot, unsigned int* destinationLength, int** destinationArray);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntArrayPropertyLength(LavObject* obj, int slot, int* destination);
//applies to eather array type.
Lav_PUBLIC_FUNCTION LavError Lav_objectGetArrayPropertyLengthRange(LavObject* obj, int slot, unsigned int* destinationMin, unsigned int* destinationMax);

/**Callbacks (events).
Some objects go further and define specialized methods that have different signatures, but these are few and very far between.*/
EXTERN_FUNCTION typedef void (*LavEventCallback)(LavObject* cause, void* userdata);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetCallbackHandler(LavObject* obj, int callback, LavEventCallback *destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetCallbackUserDataPointer(LavObject* obj, int callback, void** destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetCallback(LavObject* obj, int callback, LavEventCallback handler, void* userData);

/**Performs the object-specific reset operation.

This does not reset properties, merely internal histories and the like.  Specifically what this means depends on the object; see the manual.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectReset(LavObject* obj);

//creators for different objject types.
//also see libaudioverse3d.h.
Lav_PUBLIC_FUNCTION LavError Lav_createSineObject(LavSimulation* sim, LavObject **destination);
Lav_PUBLIC_FUNCTION LavError Lav_createFileObject(LavSimulation *sim, const char* path, LavObject **destination);
Lav_PUBLIC_FUNCTION LavError Lav_createHrtfObject(LavSimulation *simulation, const char* hrtfPath, LavObject **destination);
Lav_PUBLIC_FUNCTION LavError Lav_createMixerObject(LavSimulation* sim, unsigned int maxParents, unsigned int inputsPerParent, LavObject **destination);
Lav_PUBLIC_FUNCTION LavError Lav_createAttenuatorObject(LavSimulation *sim, unsigned int numChannels, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterObject(LavSimulation* sim, unsigned int numInputs, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_createDelayObject(LavSimulation* sim, unsigned int lines, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerObject(LavSimulation* sim, LavObject** destination);

#ifdef __cplusplus
}
#endif