/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/**The public interface to Libaudioverse.*/

/*Forward-declares all Libaudioverse types.
Enums can't go here. Everything else can.*/
#ifdef __cplusplus
class LavObject;
class LavDevice;
class LavHrtfData;
#else
typedef struct LavHrtfData LavHrtfData;
typedef struct LavDevice LavDevice;
typedef struct LavObject LavObject;
#endif

/**Does whatever is appropriate on a given platform to expose a Libaudioverse function publically.*/
#ifdef _MSC_VER
#define DLL_PUBLIC_ATTR __declspec(dllexport)
#endif
#ifndef DLL_PUBLIC_ATTR
#define DLL_PUBLIC_ATTR
#endif
#ifdef __cplusplus
#define Lav_PUBLIC_FUNCTION extern "C" DLL_PUBLIC_ATTR
#else
#define Lav_PUBLIC_FUNCTION extern DLL_PUBLIC_ATTR
#endif

/*This block takes effect if this header is being preprocessed for the tests, turning off whatever weird thing we need for the build.*/
#ifdef IS_TESTING
//gets rid of macro redefinition warnings.
#undef Lav_PUBLIC_FUNCTION
#define Lav_PUBLIC_FUNCTION
#endif

enum Lav_LIMITS {
	Lav_MAX_BLOCK_SIZE = 1024,
};
typedef enum Lav_LIMITS LavLimits;

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
};

/**Typedef for error codes.*/
typedef enum Lav_ERRORS LavError;

/**These are property types, either int, float, double, vector of 3 floats, vector of 6 floats, or string.*/
enum Lav_PROPERTYTYPES {
	Lav_PROPERTYTYPE_INT = 1,
	Lav_PROPERTYTYPE_FLOAT = 2,
	Lav_PROPERTYTYPE_DOUBLE = 3,
	Lav_PROPERTYTYPE_STRING = 4,
	Lav_PROPERTYTYPE_FLOAT3 = 5,
	Lav_PROPERTYTYPE_FLOAT6 = 6,
};

/**These are used to tag objects with their type, so that external languages may see them.*/
enum Lav_OBJTYPES {
	Lav_OBJTYPE_GENERIC, //this is not something you should ever see outside the library, and basically means none.
	Lav_OBJTYPE_WORLD,
	Lav_OBJTYPE_SOURCE_MONO,
	Lav_OBJTYPE_ZEROS,
	Lav_OBJTYPE_FILE,
	Lav_OBJTYPE_HRTF,
	Lav_OBJTYPE_SINE,
	Lav_OBJTYPE_MIXER,
	Lav_OBJTYPE_ATTENUATOR,
	Lav_OBJTYPE_HARD_LIMITER,
};

/**Initialize Libaudioverse.*/
Lav_PUBLIC_FUNCTION LavError Lav_initializeLibrary();

//devices...
Lav_PUBLIC_FUNCTION LavError Lav_createDefaultAudioOutputDevice(LavDevice** destination);

/**This type of device is intended for apps that wish to handle audio themselves: it will not output and time will not advance for it.
Combine it with Lav_deviceReadBlock to make use of it.*/
Lav_PUBLIC_FUNCTION LavError Lav_createReadDevice(unsigned int sr, unsigned int channels, unsigned int blockSize, LavDevice** destination);

Lav_PUBLIC_FUNCTION LavError Lav_deviceSetOutputObject(LavDevice* device, LavObject* object);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetOutputObject(LavDevice* device, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetBlock(LavDevice* device, float* destination);

/**Query maximum number of inputs and outputs.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputCount(LavObject* obj, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetOutputCount(LavObject* obj, unsigned int* destination);

/**Parent management.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetParent(LavObject *obj, unsigned int slot, LavObject** parent, unsigned int *outputNumber);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetParent(LavObject *obj, unsigned int input, LavObject* parent, unsigned int output);
Lav_PUBLIC_FUNCTION LavError Lav_objectClearParent(LavObject *obj, unsigned int slot);

/**Resets a property to its default value, for any type.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectResetProperty(LavObject *obj, unsigned int slot);

/**Property getters and setters.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectSetIntProperty(LavObject* obj, unsigned int slot, int value);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloatProperty(LavObject *obj, unsigned int slot, float value);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetDoubleProperty(LavObject *obj, unsigned int slot, double value);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetStringProperty(LavObject*obj, unsigned int slot, char* value);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat3Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetFloat6Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3, float v4, float v5, float v6);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetIntProperty(LavObject*obj, unsigned int slot, int *destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloatProperty(LavObject* obj, unsigned int slot, float *destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetDoubleProperty(LavObject*obj, unsigned int slot, double *destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetStringProperty(LavObject* obj, unsigned int slot, const char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat3Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetFloat6Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6);

/**Query property ranges. These are set only by internal code.  Float3 and Float6 are effectively rangeless: specifically what a range is for those is very undefined and specific to the object in question.*/
Lav_PUBLIC_FUNCTION LavError Lav_getIntPropertyRange(LavObject* obj, unsigned int slot, int* lower, int* upper);
Lav_PUBLIC_FUNCTION LavError Lav_getFloatPropertyRange(LavObject* obj, unsigned int slot, float* lower, float* upper);
Lav_PUBLIC_FUNCTION LavError Lav_getDoublePropertyRange(LavObject* obj, unsigned int slot, double* lower, double* upper);

/**Make a sine object.*/
Lav_PUBLIC_FUNCTION LavError Lav_createSineObject(LavDevice* device, LavObject **destination);

/**A object that plays a file.*/
Lav_PUBLIC_FUNCTION LavError Lav_createFileObject(LavDevice*device, const char* path, LavObject **destination);

/**Load hrtf dataset from file.  This is for use with hrtf objects, and the 3D audio API.*/
Lav_PUBLIC_FUNCTION LavError Lav_createHrtfData(const char* path, LavHrtfData **destination);

/**Make a HRTF object.*/
Lav_PUBLIC_FUNCTION LavError Lav_createHrtfObject(LavDevice* device, LavHrtfData* hrtf, LavObject **destination);

/**Make a mixer.*/
Lav_PUBLIC_FUNCTION LavError Lav_createMixerObject(LavDevice* device, unsigned int maxParents, unsigned int inputsPerParent, LavObject **destination);

/**Make an attenuator.*/
Lav_PUBLIC_FUNCTION LavError Lav_createAttenuatorObject(LavDevice* device, unsigned int numChannels, LavObject** destination);

/**A hard limiter*/
Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterObject(LavDevice* device, unsigned int numInputs, LavObject** destination);

#ifdef __cplusplus
}
#endif