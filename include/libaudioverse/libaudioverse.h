/**The public interface to Libaudioverse.*/

#include "libaudioverse_fd.h"

/**Does whatever is appropriate on a given platform to expose a Libaudioverse function publically.*/
#define Lav_PUBLIC_FUNCTION __declspec(dllexport)

enum Lav_ERRORS {
	Lav_ERROR_NONE, //everything is OK.
	Lav_ERROR_UNKNOWN, //We know something has gone wrong, but can't figure out what.
	Lav_ERROR_TYPE_MISMATCH, //Tried to get/set something with the wrong type, i.e. properties.
	Lav_ERROR_INVALID_SLOT, //one of the functions taking a slot got passed an invalid number.
	Lav_ERROR_NULL_POINTER, //you passed a NULL pointer into something that shouldn't have it.
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
	Lav_PROPERTYRESOLUTION_BLOCK = 0x1,
	Lav_PROPERTYRESOLUTION_SAMPLE = 0x2,
};

/**These are used to tag nodes with their type, so that external languages may see them.*/
enum Lav_NODETYPES{
	Lav_NODETYPE_FILE_READER,
	Lav_NODETYPE_CONVOLVER,
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
};

struct Lav_SampleBuffer_s {
	unsigned int length;
	float *samples;
	LavNode *owner;
};

struct Lav_Stream_s {
	LavSampleBuffer *associated_buffer;
	unsigned int position;
};

struct Lav_Node_s {
	LavSampleBuffer *outputs;
	unsigned int num_outputs;
	LavStream *inputs;
	unsigned int num_inputs;
	LavProperty *properties;
	unsigned int num_properties;
	enum Lav_NODETYPES type;
};

struct Lav_NodeWithHistory_s {
	LavNode base;
	unsigned int history_length;
	float* history;
};

/**Free an instance of a node.*/
Lav_PUBLIC_FUNCTION LavError freeNode(LavNode *node);

/**The following functions initialize nodes and work exactly as one would expect.
They return NULL on error.*/
Lav_PUBLIC_FUNCTION LavError Lav_makeNode(unsigned int size, unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type, LavNode **destination);
Lav_PUBLIC_FUNCTION LavError Lav_makeHistoryNode(unsigned int size, unsigned int numInputs,
	unsigned int numOutputs, enum Lav_NODETYPE type, unsigned int historyLength, LavNodeWithHistory **destination);

/**Parent management.*/
Lav_PUBLIC_FUNCTION LavError getParent(LavNode *node, unsigned int slot, LavNode** destination);
Lav_PUBLIC_FUNCTION LavError Lav_setParent(LavNode *node, LavNode *parent, unsigned int slot);
Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavNode *node, unsigned int slot);

/**Resets a property to its default value, for any type.*/
Lav_PUBLIC_FUNCTION LavError lav_resetProperty(LavNode *node, unsigned int slot);

/**Property getters and setters.*/
Lav_PUBLIC_FUNCTION LavError Lav_setIntProperty(LavNode* node, unsigned int slot, int value);
Lav_PUBLIC_FUNCTION LavError Lav_setFloatProperty(LavNode *node, unsigned int slot, float value);
Lav_PUBLIC_FUNCTION LavError Lav_setDoubleProperty(LavNode *node, unsigned int slot, double value);
Lav_PUBLIC_FUNCTION LavError Lav_setStringProperty(LavNode *node, unsigned int slot, char* value);
Lav_PUBLIC_FUNCTION LavError Lav_getIntProperty(LavNode *node, unsigned int slot, int *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getFloatProperty(LavNode* node, unsigned int slot, float *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getDoubleProperty(LavNode *node, unsigned int slot, double *destination);
Lav_PUBLIC_FUNCTION LavError Lav_getStringProperty(LavNode* node, unsigned int slot, char** destination);
