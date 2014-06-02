/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all nodes: linking, allocating, and freeing, as well as parent-child relationships.

Note: this file is heavily intertwined with stream_buffers.c, though it does not use private functionality of that file.*/
#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_all.h>
#include "graphs.h"

Lav_PUBLIC_FUNCTION LavError freeNode(LavNode *node) {
	free(node);
	return Lav_ERROR_NONE;
}

float zerobuffer[Lav_MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

void nodeComputeInputBuffers(LavNode* node) {
	//point our inputs either at a zeroed buffer or the output of our parent.
	for(unsigned int i = 0; i < node->num_inputs; i++) {
		if(node->input_descriptors[i].parent != NULL) {
			node->inputs[i] = node->input_descriptors[i].parent->outputs[node->input_descriptors[i].output];
		}
		else {
			node->inputs[i] = zerobuffer;
		}
	}
}

Lav_PUBLIC_FUNCTION LavError Lav_createNode(unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type, LavGraph *graph, LavNode **destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(graph);
	LOCK(graph->mutex);
	LavNode *retval = calloc(1, sizeof(LavNode));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;

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
		for(unsigned int i = 0; i < numOutputs; i++) {
			retval->outputs[i] = calloc(graph->block_size, sizeof(float));
			ERROR_IF_TRUE(retval->outputs[i] == NULL, Lav_ERROR_MEMORY);
		}
	}

	retval->type = type;
	retval->process = Lav_processDefault;

	nodeComputeInputBuffers(retval);

	//remember what graph we belong to, and asociate.
	retval->graph = graph;
	graphAssociateNode(retval->graph, retval);
	*destination = retval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

/*Default Processing function.*/
Lav_PUBLIC_FUNCTION LavError Lav_processDefault(LavNode *node) {
	for(unsigned int i = 0; i < node->num_outputs; i++) {
		memset(node->outputs[i], 0, node->graph->block_size*sizeof(float));
	}
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_setParent(LavNode *node, LavNode *parent, unsigned int outputSlot, unsigned int inputSlot) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	CHECK_NOT_NULL(parent);
	LOCK(node->graph->mutex);
	ERROR_IF_TRUE(inputSlot >= node->num_inputs, Lav_ERROR_INVALID_SLOT);
	ERROR_IF_TRUE(outputSlot >= parent->num_outputs, Lav_ERROR_INVALID_SLOT);
	node->input_descriptors[inputSlot].parent = parent;
	node->input_descriptors[inputSlot].output = outputSlot;
	nodeComputeInputBuffers(node);
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavNode *node, unsigned int slot, LavNode **parent, unsigned int *outputNumber) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	CHECK_NOT_NULL(parent);
	CHECK_NOT_NULL(outputNumber);
	LOCK(node->graph->mutex);
	ERROR_IF_TRUE(slot < 0 || slot >= node->num_inputs, Lav_ERROR_RANGE);
	*parent = node->input_descriptors[slot].parent;
	*outputNumber = node->input_descriptors[slot].output;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavNode *node, unsigned int slot) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	LOCK(node->graph->mutex);
	ERROR_IF_TRUE(slot >= node->num_inputs, Lav_ERROR_INVALID_SLOT);
	node->input_descriptors[slot].parent = NULL;
	node->input_descriptors[slot].output = 0;
	nodeComputeInputBuffers(node);
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}


Lav_PUBLIC_FUNCTION LavError Lav_nodeReadBlock(LavNode *node, float* destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	CHECK_NOT_NULL(destination);
	LOCK(node->graph->mutex);
	//the outputs are now read to read.  Do so.
	for(unsigned int i = 0; i < node->graph->block_size; i++) {
		for(unsigned int output = 0; output < node->num_outputs; output++) {
			destination[i*node->num_outputs+output] = node->outputs[output][i];
		}
	}
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}
