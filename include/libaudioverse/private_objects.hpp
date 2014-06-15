/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "private_threads.hpp"
#include "libaudioverse.h"

struct LavInputDescriptor {
	LavObject* parent;
	unsigned int output;
};

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
	int is_in_processor; //set to 1 by the graph processing algorithm exactly before the process method is called, and set to 0 immediately after.

	//this preventss all sorts of trouble.
	virtual ~LavObject() {}
};

LavError initLavObject(unsigned int numInputs, unsigned int numOutputs, unsigned int numProperties, LavPropertyTableEntry* propertyTable, enum  Lav_NODETYPES type, LavDevice* device, LavObject *destination);
LavError Lav_createObject(unsigned int numInputs, unsigned int numOutputs, unsigned int numProperties, LavPropertyTableEntry* propertyTable, enum  Lav_NODETYPES type, LavDevice* device, LavObject **destination);
LavError objectProcessSafe(LavObject* obj);
