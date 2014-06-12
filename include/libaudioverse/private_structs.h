/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_threads.h"
#ifdef __cplusplus
extern "C" {
#endif

//All structs here are private.
//Anything not exposed directly through the public api is followed by its typedef in this file.

//audio devices.
//the specific functionality of an audio device needs to be hidden behind the void* data parameter, but the three function pointers *must* be filled out.
//furthermore, mutex *must* be set to something and block_size must be greater than 0.
//The above assumptions are made throughout the entire library.
struct LavDevice {
	LavError (*get_block)(LavDevice* device, float* destination);
	LavError (*start)(LavDevice* device);
	LavError (*stop)(LavDevice *device);
	LavError (*kill)(LavDevice* device);
	//this one is optional.  Intended for simulations and the like that want a chance to do stuff every block.
	void (*preprocessing_hook)(LavDevice* device, void* argument);
	void* preprocessing_hook_argument;
	unsigned int block_size, channels, mixahead;
	float sr;
	void* mutex, *device_specific_data;
	struct LavObject** objects;
	unsigned object_count, max_object_count;
	struct LavObject* output_object;
};

union LavPropertyValue {
	float fval;
	int ival;
	double dval;
	char* sval;
	float f3val[3]; //vectors.
	float f6val[6]; //orientations.
};
typedef union LavPropertyValue LavPropertyValue;

struct LavObject;
typedef void (*LavPropertyChangedCallback)(struct LavObject* obj, int slot);

struct LavProperty {
	enum Lav_PROPERTYTYPES type;
	LavPropertyValue value, default_value;
	char* name;
	LavPropertyChangedCallback post_changed_callback;
};

typedef struct LavProperty LavProperty;

struct LavInputDescriptor {
	LavObject* parent;
	unsigned int output;
};
typedef struct LavInputDescriptor LavInputDescriptor;

/**Things all Libaudioverse objects have.*/
//typedef for processing function:
typedef LavError (*LavProcessorFunction)(LavObject* obj);
struct LavObject {
	LavDevice *device;
	LavProperty **properties;
	unsigned int num_properties;
	float** inputs;
	LavInputDescriptor *input_descriptors;
	float** outputs;
	unsigned int num_outputs;
	unsigned int num_inputs;
	LavProcessorFunction process; //what to call to process this node.
	void* mutex;
	enum Lav_NODETYPES type;
	int has_processed; //used for optimizations of the graph algorithm.
	int should_always_process; //if true, this node will be processed every tick regardless of if the graph algorithm finds it.
};

struct LavNode {
	LavObject base;
	double internal_time;
	void *data; //place for node subtypes to place data.
};
typedef struct LavNode LavNode;

struct LavCrossThreadRingBuffer {
	int read_position, write_position, element_size, length;
	void* lock;
	char* data;
	int last_op;
};
typedef struct LavCrossThreadRingBuffer LavCrossThreadRingBuffer;

/**A table of audio data.*/
struct LavTable {
	unsigned int length; //length in samples.  Also the max index before index wrapping.  Includes the extra sample at the end.
	float* samples;
};
typedef struct LavTable LavTable;

//The struct for property table entries.
struct LavPropertyTableEntry {
	int slot;
	enum Lav_PROPERTYTYPES type;
	char* name;
	union LavPropertyValue default_value;
	LavPropertyChangedCallback post_changed;
};
typedef struct LavPropertyTableEntry LavPropertyTableEntry;

#ifdef __cplusplus
}
#endif