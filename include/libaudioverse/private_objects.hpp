/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "private_threads.hpp"
#include "libaudioverse.h"

class LavInputDescriptor {
	public:
	LavObject* parent;
	unsigned int output;
};

/**Things all Libaudioverse objects have.*/

class LavObject {
	public:
	LavDevice *device;
	LavProperty **properties;
	unsigned int num_properties;
	float** inputs;
	LavInputDescriptor *input_descriptors;
	float** outputs;
	unsigned int num_outputs;
	unsigned int num_inputs;
	virtual LavError process();
	void* mutex;
	enum Lav_NODETYPES type;
	int has_processed; //used for optimizations of the graph algorithm.
	int should_always_process; //if true, this node will be processed every tick regardless of if the graph algorithm finds it.
	int is_in_processor; //set to 1 by the graph processing algorithm exactly before the process method is called, and set to 0 immediately after.

	//construction.
	//note that the default LavObject has no properties and that it is up to subclass constructors to add/configure them.
	LavObject(LavDevice* device, unsigned int numInputs, unsigned int numOutputs);

	//this preventss all sorts of trouble.
	virtual ~LavObject() {}

	//LavObjects are only to ever be created on the heap through expllicit specification of number of inputs and outputs, and any clone functionality must be explicit.  If a subclass wishes to implement copying it can, but there really isn't a reason.
	LavObject() = delete;
	LavObject(const LavObject&) = delete;
	LavObject& operator=(const LavObject&) = delete;
};

