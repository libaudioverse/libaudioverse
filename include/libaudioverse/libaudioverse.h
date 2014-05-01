/**The public interface to Libaudioverse.*/

#include "libaudioverse_fd.h"

/**Does whatever is appropriate on a given platform to expose a Libaudioverse function publically.*/
#define LAV_PUBLIC_FUNCTION

/**These are property types, either int, float, double, or string.

Note that they can be ored.  This is important and intended as an extension point.  Namely, some properties in future are going to allow themselves to be either a constant or an LFO.*/
enum Lav_PROPERRTYTYPES {
	Lav_PROPERTYTYPE_INT = 0x1,
	Lav_PROPERTYTYPE_FLOAT = 0x2,
	Lav_PROPERTYTYPE_DOUBLE = 0x4,
	Lav_PROPERTYTYPE_STRING = 0x8,
};

/**Whenh is this property processed?  Either block or per-sample.*/
enum lav_PROPERTYRESOLUTION {
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
