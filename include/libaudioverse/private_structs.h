#pragma once
#include "libaudioverse.h"

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

struct Lav_Node_s {
	LavSampleBuffer *outputs;
	unsigned int num_outputs;
	LavStream *inputs;
	unsigned int num_inputs;
	LavProperty *properties;
	unsigned int num_properties;
	enum Lav_NODETYPES type;
	LavNodeProcessorFunction process; //what to call to process this node.
	double internal_time;
	float sr;
	void *type_specific_data; //place for node subtypes to place data.
};
