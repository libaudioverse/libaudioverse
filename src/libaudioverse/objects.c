/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/

#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_all.h>
#include "graphs.h"

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

void objectComputeInputBuffers(LavObject* obj) {
	//point our inputs either at a zeroed buffer or the output of our parent.
	for(unsigned int i = 0; i < obj->num_inputs; i++) {
		if(obj->input_descriptors[i].parent != NULL) {
			obj->inputs[i] = obj->input_descriptors[i].parent->outputs[obj->input_descriptors[i].output];
		}
		else {
			obj->inputs[i] = zerobuffer;
		}
	}
}

LavError Lav_initObject(unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type, unsigned int blockSize, void* mutex, LavObject **destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(destination);
	CHECK_NOT_NULL(mutex);
	ERROR_IF_TRUE(blockSize > Lav_MAX_BLOCK_SIZE, Lav_ERROR_RANGE);
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;
	retval->mutex = mutex;
	//allocations:
	if(numInputs > 0) {
		retval->input_descriptors = calloc(numInputs, sizeof(LavInputDescriptor));
		ERROR_IF_TRUE(retval->input_descriptors == NULL, Lav_ERROR_MEMORY);
		retval->inputs = calloc(numInputs, sizeof(float*));
		ERROR_IF_TRUE(retval->inputs == NULL, Lav_ERROR_MEMORY);
	}

	if(numOutputs > 0) {
		retval->outputs = calloc(numOutputs, sizeof(float**));
		ERROR_IF_TRUE(retval->outputs == NULL, Lav_ERROR_MEMORY);
		for(unsigned int i = 0; i < retval->num_outputs; i++) {
			retval->outputs[i] = calloc(blockSize, sizeof(float));
			ERROR_IF_TRUE(retval->outputs[i] == NULL, Lav_ERROR_MEMORY);
		}
	}

	retval->type = type;
	objectComputeInputBuffers(retval); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.
}

Lav_PUBLIC_FUNCTION LavError Lav_createObject(unsigned int numInputs, unsigned int numOutputs, enum  Lav_OBJECTTYPE type, unsigned int blockSize, void* mutex, LavObject **destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(mutex);
	LavNode *retval = calloc(1, sizeof(LavObject));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	LavError err = LavError Lav_initObject(numInputs, numOutputs, type, blockSize, &retval);
	ERROR_IF_TRUE(err != lav_ERROR_NONE, err);
	SAFERETURN(Lav_ERROR_NONE);
	*destination = retval;
	STANDARD_CLEANUP_BLOCK;
}

/*Default Processing function.*/
Lav_PUBLIC_FUNCTION LavError Lav_processDefault(LavObject* obj) {
	for(unsigned int i = 0; i < node->num_outputs; i++) {
		memset(obj->outputs[i], 0, obj->block_size*sizeof(float));
	}
	return Lav_ERROR_NONE;
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
	objectComputeInputBuffers(obj);
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavNode *obj, unsigned int slot, LavObject **parent, unsigned int *outputNumber) {
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
	LOCK(node->mutex);
	ERROR_IF_TRUE(slot >= obj->num_inputs, Lav_ERROR_INVALID_SLOT);
	obj->input_descriptors[slot].parent = NULL;
	obj->input_descriptors[slot].output = 0;
	objectComputeInputBuffers(obj);
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}


Lav_PUBLIC_FUNCTION LavError Lav_nodeReadBlock(LavObject *what, float* destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(obj);
	CHECK_NOT_NULL(destination);
	LOCK(obj->mutex);
	//the outputs are now read to read.  Do so.
	for(unsigned int i = 0; i < node->block_size; i++) {
		for(unsigned int output = 0; output < obj->num_outputs; output++) {
			destination[i*obj->num_outputs+output] = obj->outputs[output][i];
		}
	}
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}
