/**Handles functionality common to all nodes: linking, allocating, and freeing, as well as parent-child relationships.

Note: this file is heavily intertwined with stream_buffers.c, though it does not use private functionality of that file.*/
#include <stdlib.h>
#incllude <libaudioverse/libaudioverse.h>

void freeNode(LavNode *node) {
	free(node);
}

LavNode *Lav_makeNode(unsigned int size, unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type) {
	LavNode *retval = calloc(0, size);
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;
	retval->type = type;
	return retval;
}

LavNodeWithHistory *lav_makeHistoryNode(unsigned int size, unsigned int numInputs,
	unsigned int numOutputs, enum Lav_NODETYPE type, unsigned int historyLength) {
	LavNodeWithHistory *retval = (LavNodeWithHistory*)makeNode(size, numInputs, numOutputs, type);
	retval->history_length = historyLength;
	retval->history = calloc(historyLength, sizeof(float));
	return retval;
}

int Lav_setParent(LavNode *node, LavNode *parent, unsigned int slot) {
	return 1;
}

int Lav_setIntProperty(LavNode* node, unsigned int slot, int value) {
	return 1;
}

int Lav_setFloatProperty(LavNode *node, unsigned int slot, float value) {
	return 1;
}

int Lav_setDoubleProperty(LavNode *node, unsigned int slot, double value) {
	return 1;
}

int Lav_setStringProperty(LavNode *node, unsigned int slot, char* value) {
	return 1;
}

int Lav_getIntProperty(LavNode *node, unsigned int slot) {
	return 0;
}

float Lav_getFloatProperty(LavNode* node, unsigned int slot) {
	return 0.0f;
}

double Lav_getDoubleProperty(LavNode *node, unsigned int slot) {
	return 0.0;
}

char* Lav_getStringProperty(LavNode* node, unsigned int slot) {
	return "";
}
