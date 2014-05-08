/**Handles functionality common to all nodes: linking, allocating, and freeing, as well as parent-child relationships.

Note: this file is heavily intertwined with stream_buffers.c, though it does not use private functionality of that file.*/
#include <stdlib.h>
#include <string.h>
#include <libaudioverse/private_all.h>
#include "graphs.h"
#include <SDL_mutex.h>

Lav_PUBLIC_FUNCTION LavError freeNode(LavNode *node) {
	free(node);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_createNode(unsigned int numInputs, unsigned int numOutputs, unsigned int numProperties, enum  Lav_NODETYPE type, LavGraph *graph, LavNode **destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(graph);
	LOCK(graph->mutex);
	LavNode *retval = calloc(1, sizeof(LavNode));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;
	retval->num_properties = numProperties;

	//allocations:
	if(numInputs > 0) retval->inputs = calloc(numInputs, sizeof(LavStream));
	if(numProperties > 0) retval->properties = calloc(numProperties, sizeof(LavProperty));
	if(numOutputs > 0) retval->outputs = calloc(numOutputs, sizeof(LavSampleBuffer));
	retval->type = type;
	retval->process = Lav_processDefault;

	//Initialize this node's output buffers.
	for(unsigned int i = 0; i < numOutputs; i++) {
		//Set the owned node and slot to this one.
		retval->outputs[i].owner.node = retval;
		retval->outputs[i].owner.slot = i;
		//Make its sample buffer.
		retval->outputs[i].samples = calloc(2048, sizeof(float));
		retval->outputs[i].length = 2048;
	}
	//There's nothing to do for the streams: they all point at NULL parents.
	//If we have a tleast two properties, they're add and mul.
	if(numProperties >= 2) {
		retval->properties[Lav_STDPROPERTY_ADD].name = "add";
		retval->properties[Lav_STDPROPERTY_ADD].type = Lav_PROPERTYTYPE_FLOAT;
		retval->properties[Lav_STDPROPERTY_ADD].value.fval = 0.0f;
		retval->properties[Lav_STDPROPERTY_ADD].default_value.fval = 0.0f;
		retval->properties[Lav_STDPROPERTY_MUL].type = Lav_PROPERTYTYPE_FLOAT;
		retval->properties[Lav_STDPROPERTY_MUL].value.fval = 1.0f;
		retval->properties[Lav_STDPROPERTY_MUL].default_value.fval = 1.0f;
		retval->properties[Lav_STDPROPERTY_MUL].name = "mul";
	}

	//remember what graph we belong to, and asociate.
	retval->graph = graph;
	graphAssociateNode(retval->graph, retval);
	*destination = retval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

/*Default Processing function.*/
Lav_PUBLIC_FUNCTION LavError Lav_processDefault(LavNode *node, unsigned int count) {
	for(unsigned int i = 0; i < node->num_outputs; i++) {
		for(unsigned int j = 0; j < count; j++) {
			Lav_bufferWriteSample(node->outputs+i, 0.0f);
		}
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
	LavSampleBuffer *b = &parent->outputs[outputSlot];
	LavStream *s = &node->inputs[inputSlot];
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
	if(node->inputs[slot].associated_buffer == NULL) {
		*parent = NULL;
		*outputNumber = 0;
	}
	else {
		*parent = node->inputs[slot].associated_buffer->owner.node;
		*outputNumber = node->inputs[slot].associated_buffer->owner.slot;
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
	node->inputs[slot].associated_buffer = NULL;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError lav_resetProperty(LavNode *node, unsigned int slot) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	LOCK(node->graph->mutex);
	ERROR_IF_TRUE(slot >= node->num_properties || slot < 0, Lav_ERROR_INVALID_SLOT);
	memcpy(&(node->properties[slot].value), &(node->properties[slot].default_value), sizeof(node->properties[slot].value)); //yes, really.
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

#define PROPERTY_SETTER_PREAMBLE(proptype) WILL_RETURN(LavError);\
CHECK_NOT_NULL(node);\
LOCK(node->graph->mutex);\
ERROR_IF_TRUE(slot >= node->num_properties || slot<=0, Lav_ERROR_INVALID_SLOT);\
ERROR_IF_TRUE(node->properties[slot].type != proptype, Lav_ERROR_TYPE_MISMATCH)\

Lav_PUBLIC_FUNCTION LavError Lav_setIntProperty(LavNode* node, unsigned int slot, int value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_INT);
	node->properties[slot].value.ival = value;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_setFloatProperty(LavNode *node, unsigned int slot, float value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT);
	node->properties[slot].value.fval = value;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_setDoubleProperty(LavNode *node, unsigned int slot, double value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_DOUBLE);
	node->properties[slot].value.dval = value;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_setStringProperty(LavNode *node, unsigned int slot, char* value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_STRING);
	CHECK_NOT_NULL(value);
	char* string = strdup(value);
	node->properties[slot].value.sval = string;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

#define PROPERTY_GETTER_PREAMBLE(proptype) WILL_RETURN(LavError);\
CHECK_NOT_NULL(node);\
CHECK_NOT_NULL(destination);\
LOCK(node->graph->mutex);\
ERROR_IF_TRUE(slot >= node->num_properties || slot < 0, Lav_ERROR_INVALID_SLOT);\
ERROR_IF_TRUE(proptype != node->properties[slot].type, Lav_ERROR_TYPE_MISMATCH)

Lav_PUBLIC_FUNCTION LavError Lav_getIntProperty(LavNode *node, unsigned int slot, int *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_INT);
	*destination = node->properties[slot].value.ival;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloatProperty(LavNode* node, unsigned int slot, float *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT);
	*destination = node->properties[slot].value.fval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_getDoubleProperty(LavNode *node, unsigned int slot, double *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_DOUBLE);
	*destination = node->properties[slot].value.dval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_getStringProperty(LavNode* node, unsigned int slot, char** destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_STRING);
	*destination = node->properties[slot].value.sval;
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
		streams[i].associated_buffer = &node->outputs[i];
		streams[i].position = node->outputs[i].write_position;
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
	for(unsigned int sample = 0;  sample < samples; sample++) {
		for(unsigned int output = 0; output < node->num_outputs; output++) {
			destination[(output+1)*sample] = output_array[output][sample]; //the +1 is necessary.
		}
	}

	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}