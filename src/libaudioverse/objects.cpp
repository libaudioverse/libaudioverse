/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/

#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_all.hpp>

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

void computeInputBuffers() {
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
		input_descriptors = calloc(numInputs, sizeof(LavInputDescriptor));
		inputs = calloc(numInputs, sizeof(float*));
	}

	if(numOutputs > 0) {
		outputs = calloc(numOutputs, sizeof(float**));
		for(unsigned int i = 0; i < num_outputs; i++) {
			outputs[i] = calloc(device->block_size, sizeof(float));
		}
	}
	deviceAssociateObject(device, destination);
	computeInputBuffers(); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.
}


/*Default Processing function.*/
void LavObject::process() {
	for(unsigned int i = 0; i < num_outputs; i++) {
		memset(outputs[i], 0, obj->device->block_size*sizeof(float));
	}
}

Lav_PUBLIC_FUNCTION LavError Lav_setParent(LavObject *obj, LavObject*parent, unsigned int outputSlot, unsigned int inputSlot) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(obj);
	CHECK_NOT_NULL(parent);
	LOCK(obj->mutex);
	ERROR_IF_TRUE(inputSlot >= obj->num_inputs, Lav_ERROR_INVALID_SLOT);
	ERROR_IF_TRUE(outputSlot >= parent->num_outputs, Lav_ERROR_INVALID_SLOT);
	obj->input_descriptors[inputSlot].parent = parent;
	obj->input_descriptors[inputSlot].output = outputSlot;
	obj->computeInputBuffers();
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavObject *obj, unsigned int slot, LavObject **parent, unsigned int *outputNumber) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(obj);
	CHECK_NOT_NULL(parent);
	CHECK_NOT_NULL(outputNumber);
	LOCK(obj->mutex);
	ERROR_IF_TRUE(slot < 0 || slot >= obj->num_inputs, Lav_ERROR_RANGE);
	*parent = obj->input_descriptors[slot].parent;
	*outputNumber = obj->input_descriptors[slot].output;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavObject *obj, unsigned int slot) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(obj);
	LOCK(obj->mutex);
	ERROR_IF_TRUE(slot >= obj->num_inputs, Lav_ERROR_INVALID_SLOT);
	obj->input_descriptors[slot].parent = NULL;
	obj->input_descriptors[slot].output = 0;
	obj->computeInputBuffers();
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}
