/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_threads.h"

//All structs here are private.
//Anything not exposed indireclty through the public api is followed by its typedef in this file.

union Lav_PropertyValue_u {
	float fval;
	int ival;
	double dval;
	char* sval;
};

struct Lav_Property_s {
	enum Lav_PROPERTYTYPES type;
	enum Lav_PROPERTYRESOLUTION resolution;
	union Lav_PropertyValue_u value, default_value;
	char* name;
};

struct Lav_Node_s {
	LavGraph *graph;
	unsigned int num_outputs;
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

//The struct for property table entries.
struct Lav_PropertyTableEntry_s {
	int slot;
	enum Lav_PROPERTYTYPES type;
	char* name;
	union Lav_PropertyValue_u default_value;
};

typedef struct Lav_PropertyTableEntry_s LavPropertyTableEntry;
