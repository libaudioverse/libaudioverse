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
typedef struct LavHrtfData LavHrtfData;
typedef struct LavObject LavObject;

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

	//these are for hrtf.
	Lav_ERROR_HRTF_CORRUPT = 10,
	Lav_ERROR_HRTF_TOO_SMALL = 11,
	Lav_ERROR_HRTF_INVALID = 12,
};

/**Typedef for error codes.*/
typedef enum Lav_ERRORS LavError;

/**These are property types, either int, float, double, or string.

Note that they can be ored.  This is important and intended as an extension point.  Namely, some properties in future are going to allow themselves to be either a constant or an LFO.*/
enum Lav_PROPERTYTYPES {
	Lav_PROPERTYTYPE_INT = 0x1,
	Lav_PROPERTYTYPE_FLOAT = 0x2,
	Lav_PROPERTYTYPE_DOUBLE = 0x4,
	Lav_PROPERTYTYPE_STRING = 0x8,
	Lav_PROPERTYTYPE_FLOAT3 = 0xf,
	Lav_PROPERTYTYPE_FLOAT6 = 0x10,
};

/**These are used to tag nodes with their type, so that external languages may see them.*/
enum Lav_NODETYPES{
	Lav_OBJTYPE_GRAPH,
	Lav_NODETYPE_ZEROS,
	Lav_NODETYPE_FILE,
	Lav_NODETYPE_HRTF,
	Lav_NODETYPE_SINE,
	Lav_NODETYPE_MIXER,
	Lav_NODETYPE_ATTENUATOR,
};

/**Initialize Libaudioverse.*/
Lav_PUBLIC_FUNCTION LavError Lav_initializeLibrary();


/**Graph manipulation functions: creation, deletion, and configuration.

All nodes must belong to a graph and exactly one node must be the "output node", the node which will be read from to determine a graph's output.*/
Lav_PUBLIC_FUNCTION LavError Lav_createGraph(float sr, unsigned int blockSize, LavObject **destination);
Lav_PUBLIC_FUNCTION LavError Lav_graphGetOutputNode(LavObject *graph, LavObject **destination);
Lav_PUBLIC_FUNCTION LavError Lav_graphSetOutputNode(LavObject *graph, LavObject *node);
Lav_PUBLIC_FUNCTION LavError Lav_graphGetBlock(LavObject* graph, float* samples);

/**Parent management.*/
Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavObject *obj, unsigned int slot, LavObject** parent, unsigned int *outputNumber);
Lav_PUBLIC_FUNCTION LavError Lav_setParent(LavObject *obj, LavObject *parent, unsigned int outputSlot, unsigned int inputSlot);
Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavObject *obj, unsigned int slot);

/**Resets a property to its default value, for any type.*/
Lav_PUBLIC_FUNCTION LavError Lav_resetProperty(LavObject *obj, unsigned int slot);

/**Property getters and setters.*/
Lav_PUBLIC_FUNCTION LavError Lav_setIntProperty(LavObject* obj, unsigned int slot, int value);
Lav_PUBLIC_FUNCTION LavError Lav_setFloatProperty(LavObject *obj, unsigned int slot, float value);
Lav_PUBLIC_FUNCTION LavError Lav_setDoubleProperty(LavObject *obj, unsigned int slot, double value);
Lav_PUBLIC_FUNCTION LavError Lav_setStringProperty(LavObject*obj, unsigned int slot, char* value);
Lav_PUBLIC_FUNCTION LavError Lav_setFloat3Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3);
Lav_PUBLIC_FUNCTION LavError Lav_setFloat6Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3, float v4, float v5, float v6);
Lav_PUBLIC_FUNCTION LavError Lav_getIntProperty(LavObject*obj, unsigned int slot, int *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getFloatProperty(LavObject* obj, unsigned int slot, float *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getDoubleProperty(LavObject*obj, unsigned int slot, double *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getStringProperty(LavObject* obj, unsigned int slot, char** destination);
Lav_PUBLIC_FUNCTION LavError Lav_getFloat3Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3);
Lav_PUBLIC_FUNCTION LavError Lav_getFloat6Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6);
/**This is a default node processing function. It simply writes 0s to all outputs, and can be useful when you need to provide audio and have nothing to do.*/
Lav_PUBLIC_FUNCTION LavError Lav_processDefault(LavObject *obj);

/**Helper function to read all of a node's outputs at once.
This is intended for the ability to do audio output.  The destination parameter should
be long enough to hold obj->block_size*node->num_output samples.  The samples are interweaved in an appropriate manner for most audio output systems:
the first sample of the first output, the first of the second, the first of the third,...the second of the first, second and third..., etc.
Put another way, this function pretends that the passed node has node->num_outputs channels, and then interweaves them.*/
Lav_PUBLIC_FUNCTION LavError Lav_objectReadBlock(LavObject *node, float* destination);

/**Make a sine node.*/
Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavObject *graph, LavObject **destination);

/**A node that plays a file.*/
Lav_PUBLIC_FUNCTION LavError Lav_createFileNode(LavObject *graph, const char* path, LavObject **destination);

/**Load hrtf dataset from file.  This is for use with hrtf nodes.*/
Lav_PUBLIC_FUNCTION LavError Lav_createHrtfData(const char* path, LavHrtfData **destination);

/**Make a HRTF node.*/
Lav_PUBLIC_FUNCTION LavError Lav_createHrtfNode(LavObject *graph, LavHrtfData* hrtf, LavObject **destination);

/**Make a mixer.*/
Lav_PUBLIC_FUNCTION LavError Lav_createMixerNode(LavObject *graph, unsigned int maxParents, unsigned int inputsPerParent, LavObject **destination);

/**Make an attenuator.*/
Lav_PUBLIC_FUNCTION LavError Lav_createAttenuatorNode(LavObject* graph, unsigned int numInputs, LavObject** destination);

#ifdef __cplusplus
}
#endif