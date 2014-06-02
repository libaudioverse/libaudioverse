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

float zerobuffer[MAX_BLOCK_SIZE] = {0}; //this is a shared buffer for the "no parent" case.

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
		retval->inputs = calloc(numInputs, sizeof(LavInputDesccriptor));
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
		memset(node->outputs[i], 0, block_size);
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
	//We just connect the buffers, and set the read position of the stream to the write position of the buffer.
	LavSampleBuffer *b = parent->outputs[outputSlot];
	LavStream *s = node->inputs[inputSlot];
	//Associate the stream to the buffer:
	s->associated_buffer = b;
	//And set its read position.
	s->position = b->write_position;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_getParent(LavNode *node, unsigned int slot, LavNode **parent, unsigned int *outputNumber) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	CHECK_NOT_NULL(parent);
	CHECK_NOT_NULL(outputNumber);
	if(node->inputs[slot]->associated_buffer == NULL) {
		*parent = NULL;
		*outputNumber = 0;
	}
	else {
		*parent = node->inputs[slot]->associated_buffer->owner.node;
		*outputNumber = node->inputs[slot]->associated_buffer->owner.slot;
	}
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_clearParent(LavNode *node, unsigned int slot) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	LOCK(node->graph->mutex);
	ERROR_IF_TRUE(slot >= node->num_inputs, Lav_ERROR_INVALID_SLOT);
	//This is as simple as it looks.
	node->inputs[slot]->associated_buffer = NULL;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}


Lav_PUBLIC_FUNCTION LavError Lav_nodeReadAllOutputs(LavNode *node, unsigned int samples, float* destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	CHECK_NOT_NULL(destination);
	LOCK(node->graph->mutex);

	//Make an array of LavStreams of the appropriate size.
	LavStream *streams = calloc(node->num_outputs, sizeof(LavStream));
	ERROR_IF_TRUE(streams == NULL, Lav_ERROR_MEMORY);

	//point the streams at the node.
	for(unsigned int i = 0; i < node->num_outputs; i++) {
		streams[i].associated_buffer = node->outputs[i];
		streams[i].position = node->outputs[i]->write_position;
	}

	//Now we need some output space.
	float** output_array = calloc(samples*node->num_outputs, sizeof(float*));
	ERROR_IF_TRUE(output_array == NULL, Lav_ERROR_MEMORY);

	//Fill this up with arrays of read samples.
	for(unsigned int i = 0; i < node->num_outputs; i++) {
		output_array[i] = calloc(samples, sizeof(float));
		ERROR_IF_TRUE(output_array[i] == NULL, Lav_ERROR_MEMORY);
		LavError err = Lav_streamReadSamples(&streams[i], samples, output_array[i]);
		ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	}

	//Copy to the output buffer.

	for(unsigned int i = 0; i < node->num_outputs; i++) {
		for(unsigned int j = 0; j < samples; j++) {
			destination[i+node->num_outputs*j] = output_array[i][j];
		}
	}

	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}