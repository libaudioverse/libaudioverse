/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

/**The public interface to Libaudioverse.*/

/*Forward-declares all Libaudioverse types.
Enums can't go here. Everything else can.*/

typedef struct Lav_Property_s LavProperty;
typedef struct Lav_SampleBuffer_s LavSampleBuffer;
typedef struct Lav_Node_s LavNode;
typedef struct Lav_Stream_s LavStream;
typedef struct Lav_Graph_s LavGraph;
typedef struct Lav_Table_s LavTable;
typedef struct LavHrtfData LavHrtfData;

/**Does whatever is appropriate on a given platform to expose a Libaudioverse function publically.*/
#define Lav_PUBLIC_FUNCTION extern __declspec(dllexport)

enum Lav_ERRORS {
	Lav_ERROR_NONE, //everything is OK.
	Lav_ERROR_UNKNOWN, //We know something has gone wrong, but can't figure out what.
	Lav_ERROR_TYPE_MISMATCH, //Tried to get/set something with the wrong type, i.e. properties.
	Lav_ERROR_INVALID_SLOT, //one of the functions taking a slot got passed an invalid number.
	Lav_ERROR_NULL_POINTER, //you passed a NULL pointer into something that shouldn't have it.
	Lav_ERROR_MEMORY, //a memory problem which probably isn't the fault of the application.
	Lav_ERROR_RANGE, //out of range function parameter.
	Lav_ERROR_CANNOT_INIT_AUDIO, //We couldn't even initialize the audio library.
	Lav_ERROR_FILE, //error to do with files.
	Lav_ERROR_FILE_NOT_FOUND, //specificaly, we couldn't find a file.

	//these are for hrtf.
	Lav_ERROR_HRTF_CORRUPT,
	Lav_ERROR_HRTF_TOO_SMALL,
	Lav_ERROR_HRTF_INVALID,
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
};

/**These are used to tag nodes with their type, so that external languages may see them.*/
enum Lav_NODETYPES{
	Lav_NODETYPE_ZEROS,
	Lav_NODETYPE_FILE,
	Lav_NODETYPE_CONVOLVER,
	Lav_NODETYPE_SINE,
};

/**This is the processing function's typedef.  See external documentation for info on writing your own nodes.*/
typedef LavError (*LavNodeProcessorFunction)(LavNode* node, unsigned int samples);

/**Graph manipulation functions: creation, deletion, and configuration.

All nodes must belong to a graph and exactly one node must be the "output node", the node which will be read from to determine a graph's output.*/
Lav_PUBLIC_FUNCTION LavError Lav_createGraph(float sr, LavGraph **destination);
Lav_PUBLIC_FUNCTION LavError Lav_freeGraph(LavGraph *graph);
Lav_PUBLIC_FUNCTION LavError Lav_graphGetOutputNode(LavGraph *graph, LavNode **destination);
Lav_PUBLIC_FUNCTION LavError Lav_graphSetOutputNode(LavGraph *graph, LavNode *node);

/**Works the same as Lav_nodeReadAllOutputs, below.*/
Lav_PUBLIC_FUNCTION LavError Lav_graphReadAllOutputs(LavGraph *graph, unsigned int samples, float *destination);

/**Free an instance of a node.*/
Lav_PUBLIC_FUNCTION LavError Lav_freeNode(LavNode *node);

/**The following functions initialize nodes and work exactly as one would expect.*/
Lav_PUBLIC_FUNCTION LavError Lav_createNode(unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type, LavGraph *graph, LavNode **destination);

/**Parent management.*/
Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavNode *node, unsigned int slot, LavNode** parent, unsigned int *outputNumber);
Lav_PUBLIC_FUNCTION LavError Lav_setParent(LavNode *node, LavNode *parent, unsigned int outputSlot, unsigned int inputSlot);
Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavNode *node, unsigned int slot);

/**Resets a property to its default value, for any type.*/
Lav_PUBLIC_FUNCTION LavError Lav_resetProperty(LavNode *node, unsigned int slot);

/**Property getters and setters.*/
Lav_PUBLIC_FUNCTION LavError Lav_setIntProperty(LavNode* node, unsigned int slot, int value);
Lav_PUBLIC_FUNCTION LavError Lav_setFloatProperty(LavNode *node, unsigned int slot, float value);
Lav_PUBLIC_FUNCTION LavError Lav_setDoubleProperty(LavNode *node, unsigned int slot, double value);
Lav_PUBLIC_FUNCTION LavError Lav_setStringProperty(LavNode *node, unsigned int slot, char* value);
Lav_PUBLIC_FUNCTION LavError Lav_getIntProperty(LavNode *node, unsigned int slot, int *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getFloatProperty(LavNode* node, unsigned int slot, float *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getDoubleProperty(LavNode *node, unsigned int slot, double *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getStringProperty(LavNode* node, unsigned int slot, char** destination);

/**This is a default node processing function. It simply writes 0s to all outputs, and can be useful when you need to provide audio and have nothing to do.*/
Lav_PUBLIC_FUNCTION LavError Lav_processDefault(LavNode *node, unsigned int count);

/**The following functions write and read streams.*/
Lav_PUBLIC_FUNCTION LavError Lav_bufferWriteSample(LavSampleBuffer *buffer, float sample);
Lav_PUBLIC_FUNCTION LavError Lav_streamReadSamples(LavStream *stream, unsigned int count, float *destination);

/**Helper function to read all of a node's outputs at once.
This is intended for the ability to do audio output.  The destination parameter should
be long enough to hold node->num_outputs*samples.  The samples are interweaved in an appropriate manner for most audio output systems:
the first sample of the first output, the first of the second, the first of the third,...the second of the first, second and third..., etc.
Put another way, this function pretends that the passed node has node->num_outputs channels, and then interweaves them.*/
Lav_PUBLIC_FUNCTION LavError Lav_nodeReadAllOutputs(LavNode *node, unsigned int samples, float* destination);

/**Interpolated tables.

An interpolated table has some properties of ringbuffers, but with floating point math: reads past the end or before the beginning wrap.
It is possible to read between samples; if so, it performs linear interpolation.  All times are in samples, but fractional values are allowed.*/
Lav_PUBLIC_FUNCTION LavError Lav_createTable(LavTable** destination);
Lav_PUBLIC_FUNCTION LavError Lav_tableGetSample(LavTable *table, float index, float* destination);
Lav_PUBLIC_FUNCTION LavError Lav_tableGetSamples(LavTable* table, float index, float delta, unsigned int count, float* destination);
Lav_PUBLIC_FUNCTION LavError Lav_tableSetSamples(LavTable *table, unsigned int count, float* samples);
Lav_PUBLIC_FUNCTION LavError Lav_tableClear(LavTable *table);

/**Make a sine node.*/
Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavGraph *graph, LavNode **destination);

/**A node that plays a file.*/
Lav_PUBLIC_FUNCTION LavError Lav_createFileNode(LavGraph *graph, const char* path, LavNode **destination);

/**Load hrtf dataset from file.  This is for use with panner nodes.*/
Lav_PUBLIC_FUNCTION LavError Lav_createHrtfData(const char* path, LavHrtfData **destination);
