/**Handles functionality common to all nodes: linking, allocating, and freeing, as well as parent-child relationships.

Note: this file is heavily intertwined with stream_buffers.c, though it does not use private functionality of that file.*/
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>

LAV_PUBLIC_FUNCTION LavError freeNode(LavNode *node) {
	free(node);
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_makeNode(unsigned int size, unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type, LavNode **destination) {
	LavNode *retval = calloc(0, size);
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;
	retval->type = type;
	*destination = retval;
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_makeNodeWithHistory(unsigned int size, unsigned int numInputs,
	unsigned int numOutputs, enum Lav_NODETYPE type, unsigned int historyLength, LavNodeWithHistory **destination) {
	LavNodeWithHistory* retval;
	Lav_makeNode(size, numInputs, numOutputs, type, &retval);
	retval->history_length = historyLength;
	retval->history = calloc(historyLength, sizeof(float));
	*destination = retval;
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_setParent(LavNode *node, LavNode *parent, unsigned int slot) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError getParent(LavNode *node, unsigned int slot, LavNode **destination) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_clearParent(LavNode *node, unsigned int slot) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_setIntProperty(LavNode* node, unsigned int slot, int value) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_setFloatProperty(LavNode *node, unsigned int slot, float value) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_setDoubleProperty(LavNode *node, unsigned int slot, double value) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_setStringProperty(LavNode *node, unsigned int slot, char* value) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_getIntProperty(LavNode *node, unsigned int slot, int *destination) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_getFloatProperty(LavNode* node, unsigned int slot, float *destination) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_getDoubleProperty(LavNode *node, unsigned int slot, double *destination) {
	return LAV_ERROR_NONE;
}

LAV_PUBLIC_FUNCTION LavError Lav_getStringProperty(LavNode* node, unsigned int slot, char** destination) {
	return LAV_ERROR_NONE;
}
