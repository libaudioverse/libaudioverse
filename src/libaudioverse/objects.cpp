/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/

#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_devices.hpp>

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

void LavObject::computeInputBuffers() {
	//point our inputs either at a zeroed buffer or the output of our parent.
	for(unsigned int i = 0; i < num_inputs; i++) {
		if(input_descriptors[i].parent != NULL) {
			inputs[i] = input_descriptors[i].parent->outputs[input_descriptors[i].output];
		}
		else {
			inputs[i] = zerobuffer;
		}
	}
}

LavObject::LavObject(LavDevice* device, unsigned int numInputs, unsigned int numOutputs) {
	num_inputs = numInputs;
	num_outputs = numOutputs;
	mutex = device->mutex;
	//allocations:
	if(numInputs > 0) {
		input_descriptors = new LavInputDescriptor[numInputs];
		inputs = new float*[numInputs];
	}

	if(numOutputs > 0) {
		outputs = new float*[numOutputs];
		for(unsigned int i = 0; i < num_outputs; i++) {
			outputs[i] = new float[device->block_size];
		}
	}
	device->associateObject(this);
	this->device = device;
	computeInputBuffers(); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.
}

/*Default Processing function.*/
void LavObject::process() {
	for(unsigned int i = 0; i < num_outputs; i++) {
		memset(outputs[i], 0, device->block_size*sizeof(float));
	}
}

void LavObject::setParent(unsigned int input, LavObject* parent, unsigned int parentOutput) {
	input_descriptors[input].parent = parent;
	input_descriptors[input].output = parentOutput;
	computeInputBuffers();
}

LavObject* LavObject::getParentObject(unsigned int slot) {
	return input_descriptors[slot].parent;
}

unsigned int LavObject::getParentOutput(unsigned int slot) {
	return input_descriptors[slot].output;
}

void LavObject::clearParent(unsigned int slot) {
	input_descriptors[slot].parent = nullptr;
	input_descriptors[slot].output = 0;
}

Lav_PUBLIC_FUNCTION LavError Lav_setParent(LavObject *obj, LavObject*parent, unsigned int outputSlot, unsigned int inputSlot) {
	obj->setParent(inputSlot, parent, outputSlot);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavObject *obj, unsigned int slot, LavObject **parent, unsigned int *outputNumber) {
	*parent = obj->getParentObject(slot);
	*outputNumber = obj->getParentOutput(slot);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavObject *obj, unsigned int slot) {
	obj->clearParent(slot);
	return Lav_ERROR_NONE;
}
