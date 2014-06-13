/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/

#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_all.h>

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

Lav_PUBLIC_FUNCTION LavError initLavObject(unsigned int numInputs, unsigned int numOutputs, unsigned int numProperties, LavPropertyTableEntry* propertyTable, enum  Lav_NODETYPES type, LavDevice* device, LavObject *destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(destination);
	CHECK_NOT_NULL(device);
	destination->num_inputs = numInputs;
	destination->num_outputs = numOutputs;
	destination->mutex = device->mutex;
	//allocations:
	if(numInputs > 0) {
		destination->input_descriptors = calloc(numInputs, sizeof(LavInputDescriptor));
		ERROR_IF_TRUE(destination->input_descriptors == NULL, Lav_ERROR_MEMORY);
		destination->inputs = calloc(numInputs, sizeof(float*));
		ERROR_IF_TRUE(destination->inputs == NULL, Lav_ERROR_MEMORY);
	}

	if(numOutputs > 0) {
		destination->outputs = calloc(numOutputs, sizeof(float**));
		ERROR_IF_TRUE(destination->outputs == NULL, Lav_ERROR_MEMORY);
		for(unsigned int i = 0; i < destination->num_outputs; i++) {
			destination->outputs[i] = calloc(device->block_size, sizeof(float));
			ERROR_IF_TRUE(destination->outputs[i] == NULL, Lav_ERROR_MEMORY);
		}
	}

	destination->type = type;

	if(numProperties > 0 && propertyTable != NULL) {
		LavProperty** tbl = makePropertyArrayFromTable(numProperties, propertyTable);
		ERROR_IF_TRUE(tbl == NULL, Lav_ERROR_MEMORY);
		destination->num_properties = numProperties;
		destination->properties = tbl;
	}

	LavError err = deviceAssociateObject(device, destination);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	objectComputeInputBuffers(destination); //at the moment, this is going to just make them all 0, but it takes effect once parents are added.
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_createObject(unsigned int numInputs, unsigned int numOutputs, unsigned int numProperties, LavPropertyTableEntry *propertyTable, enum  Lav_NODETYPES type, LavDevice* device, LavObject **destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(device);
	LavObject *retval = calloc(1, sizeof(LavObject));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	LavError err = initLavObject(numInputs, numOutputs, numProperties, propertyTable, type, device, retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

/*Default Processing function.*/
Lav_PUBLIC_FUNCTION LavError Lav_processDefault(LavObject* obj) {
	for(unsigned int i = 0; i < obj->num_outputs; i++) {
		memset(obj->outputs[i], 0, obj->device->block_size*sizeof(float));
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
	objectComputeInputBuffers(obj);
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}
