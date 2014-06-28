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
class LavDevice;
class LavHrtfData;
#else
typedef void LavHrtfData;
typedef void LavDevice;
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
#else
#define Lav_PUBLIC_FUNCTION extern DLL_PUBLIC_ATTR
#endif

/*This block takes effect if this header is being preprocessed for the binding generators, turning off whatever weird thing we need for the build.*/
#ifdef IS_BINDING
//gets rid of macro redefinition warnings.
#undef Lav_PUBLIC_FUNCTION
#define Lav_PUBLIC_FUNCTION
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
};

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
	Lav_OBJTYPE_GENERIC = 0, //this is not something you should ever see outside the library, and basically means none.
	Lav_OBJTYPE_WORLD = 1,
	Lav_OBJTYPE_SOURCE = 2,
	Lav_OBJTYPE_ZEROS = 3,
	Lav_OBJTYPE_FILE = 4,
	Lav_OBJTYPE_HRTF = 5,
	Lav_OBJTYPE_SINE = 6,
	Lav_OBJTYPE_MIXER = 7,
	Lav_OBJTYPE_ATTENUATOR = 8,
	Lav_OBJTYPE_HARD_LIMITER = 9,
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
Lav_PUBLIC_FUNCTION LavError Lav_deviceGetBlock(LavDevice* device, float* buffer);

/**Query maximum number of inputs and outputs.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetInputCount(LavObject* obj, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetOutputCount(LavObject* obj, unsigned int* destination);

/**Parent management.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectGetParentObject(LavObject *obj, unsigned int slot, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectGetParentOutput(LavObject* obj, unsigned int slot, unsigned int* destination);
Lav_PUBLIC_FUNCTION LavError Lav_objectSetParent(LavObject *obj, unsigned int input, LavObject* parent, unsigned int output);
Lav_PUBLIC_FUNCTION LavError Lav_objectClearParent(LavObject *obj, unsigned int slot);

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