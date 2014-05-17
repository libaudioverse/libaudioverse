#pragma once
#include "libaudioverse.h"
#include "private_threads.h"

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
	LavGraph *graph;
	LavSampleBuffer **outputs;
	unsigned int num_outputs;
	LavStream **inputs;
	unsigned int num_inputs;
	LavProperty **properties;
	unsigned int num_properties;
	enum Lav_NODETYPES type;
	LavNodeProcessorFunction process; //what to call to process this node.
	double internal_time;
	void *data; //place for node subtypes to place data.
};

struct Lav_Graph_s {
	LavNode **nodes;
	unsigned int node_count, nodes_length;
	LavNode *output_node;
	float sr; //sampling rate.
	void* mutex; //lock this graph.
	void* audio_thread; //not null thread handle for audio output graphs, otherwise null.
};

struct Lav_CrossThreadRingBuffer_s {
	int read_position, write_position, element_size, length;
	void* lock;
	char* data;
	int last_op;
};

/**A table of audio data.*/
struct Lav_Table_s {
	unsigned int length; //length in samples.  Also the max index before index wrapping.  Includes the extra sample at the end.
	float* samples;
};
