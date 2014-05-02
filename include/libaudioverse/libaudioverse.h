#pragma once

/**The public interface to Libaudioverse.*/

/*Forward-declares all Libaudioverse types.

Search for begin_meaningful_content to skip.

Enums can't go here. Everything else can.*/

struct Lav_Property_s;
typedef struct Lav_Property_s LavProperty;

struct Lav_SampleBuffer_s;
typedef struct Lav_SampleBuffer_s LavSampleBuffer;

struct Lav_Node_s;
typedef struct Lav_Node_s LavNode;

struct Lav_Stream_s;
typedef struct Lav_Stream_s LavStream;

struct Lav_NodeWithHistory_s;
typedef struct Lav_NodeWithHistory_s LavNodeWithHistory;

//begin_meaningful_content

/**Does whatever is appropriate on a given platform to expose a Libaudioverse function publically.*/
#define Lav_PUBLIC_FUNCTION extern __declspec(dllexport)

enum Lav_ERRORS {
	Lav_ERROR_NONE, //everything is OK.
	Lav_ERROR_UNKNOWN, //We know something has gone wrong, but can't figure out what.
	Lav_ERROR_TYPE_MISMATCH, //Tried to get/set something with the wrong type, i.e. properties.
	Lav_ERROR_INVALID_SLOT, //one of the functions taking a slot got passed an invalid number.
	Lav_ERROR_NULL_POINTER, //you passed a NULL pointer into something that shouldn't have it.
	Lav_ERROR_MEMORY, //a memory problem which probably isn't the fault of the application.
};

/**Typedef for error codes.*/
typedef enum Lav_ERRORS LavError;

/**These are property types, either int, float, double, or string.

Note that they can be ored.  This is important and intended as an extension point.  Namely, some properties in future are going to allow themselves to be either a constant or an LFO.*/
enum Lav_PROPERRTYTYPES {
	Lav_PROPERTYTYPE_INT = 0x1,
	Lav_PROPERTYTYPE_FLOAT = 0x2,
	Lav_PROPERTYTYPE_DOUBLE = 0x4,
	Lav_PROPERTYTYPE_STRING = 0x8,
};

/**Whenh is this property processed?  Either block or per-sample.*/
enum Lav_PROPERTYRESOLUTION {
	Lav_PROPERTYRESOLUTION_BLOCK = 0x0,
	Lav_PROPERTYRESOLUTION_SAMPLE = 0x1,
};

/**These are used to tag nodes with their type, so that external languages may see them.*/
enum Lav_NODETYPES{
	Lav_NODETYPE_ZEROS,
	Lav_NODETYPE_FILE_READER,
	Lav_NODETYPE_CONVOLVER,
	Lav_NODETYPE_SINE,
};

struct Lav_Property_s {
	enum Lav_PROPERTYTYPE type;
	enum Lav_PROPERTYRESOLUTION resolution;
	union {
		int ival;
		float fval;
		double dval;
		char* sval;
	} value, default_value;
	char* name;
};

/**This is actually a ring buffer, but should be accessed only through the public interface.*/
struct Lav_SampleBuffer_s {
	unsigned int length, write_position;
	float *samples;
	struct {
		LavNode *node;
		unsigned int slot;
	} owner;
};

struct Lav_Stream_s {
	LavSampleBuffer *associated_buffer;
	unsigned int position;
};

/**This is the processing function's typedef.  See external documentation for info on writing your own nodes.*/
typedef LavError (*LavNodeProcessorFunction)(LavNode* node, unsigned int samples);

struct Lav_Node_s {
	LavSampleBuffer *outputs;
	unsigned int num_outputs;
	LavStream *inputs;
	unsigned int num_inputs;
	LavProperty *properties;
	unsigned int num_properties;
	enum Lav_NODETYPES type;
	LavNodeProcessorFunction process; //what to call to process this node.
	float internal_time;
	float sr;
};

struct Lav_NodeWithHistory_s {
	LavNode base;
	unsigned int history_length;
	float* history;
};

/**Free an instance of a node.*/
Lav_PUBLIC_FUNCTION LavError freeNode(LavNode *node);

/**The following functions initialize nodes and work exactly as one would expect.*/
Lav_PUBLIC_FUNCTION LavError Lav_makeNode(unsigned int size, unsigned int numInputs, unsigned int numOutputs, unsigned int numProperties, enum  Lav_NODETYPE type, LavNode **destination);

/**Parent management.*/
Lav_PUBLIC_FUNCTION LavError getParent(LavNode *node, unsigned int slot, LavNode** parent, unsigned int *outputNumber);
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
Lav_PUBLIC_FUNCTION Lav_streamReadSamples(LavStream *stream, unsigned int count, float *destination);


/**Make a sine node.*/
Lav_PUBLIC_FUNCTION LavError Lav_makeSineNode(LavNode **destination);
