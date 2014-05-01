/**Handles functionality common to all nodes: linking, allocating, and freeing, as well as parent-child relationships.

Note: this file is heavily intertwined with stream_buffers.c, though it does not use private functionality of that file.*/
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>

LAV_PUBLIC_FUNCTION void freeNode(LavNode *node) {
	free(node);
}

LAV_PUBLIC_FUNCTION LavNode *Lav_makeNode(unsigned int size, unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type) {
	LavNode *retval = calloc(0, size);
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;
	retval->type = type;
	return retval;
}

LAV_PUBLIC_FUNCTION LavNodeWithHistory *lav_makeHistoryNode(unsigned int size, unsigned int numInputs,
	unsigned int numOutputs, enum Lav_NODETYPE type, unsigned int historyLength) {
	LavNodeWithHistory *retval = (LavNodeWithHistory*)Lav_makeNode(size, numInputs, numOutputs, type);
	retval->history_length = historyLength;
	retval->history = calloc(historyLength, sizeof(float));
	return retval;
}

LAV_PUBLIC_FUNCTION int Lav_setParent(LavNode *node, LavNode *parent, unsigned int slot) {
	return 1;
}

LAV_PUBLIC_FUNCTION LavNode* getParent(LavNode *node, unsigned int slot) {
	return NULL;
}

LAV_PUBLIC_FUNCTION int Lav_clearParent(LavNode *node, unsigned int slot) {
	return 0;
}

LAV_PUBLIC_FUNCTION int Lav_setIntProperty(LavNode* node, unsigned int slot, int value) {
	return 1;
}

LAV_PUBLIC_FUNCTION int Lav_setFloatProperty(LavNode *node, unsigned int slot, float value) {
	return 1;
}

LAV_PUBLIC_FUNCTION int Lav_setDoubleProperty(LavNode *node, unsigned int slot, double value) {
	return 1;
}

LAV_PUBLIC_FUNCTION int Lav_setStringProperty(LavNode *node, unsigned int slot, char* value) {
	return 1;
}

LAV_PUBLIC_FUNCTION int Lav_getIntProperty(LavNode *node, unsigned int slot) {
	return 0;
}

LAV_PUBLIC_FUNCTION float Lav_getFloatProperty(LavNode* node, unsigned int slot) {
	return 0.0f;
}

LAV_PUBLIC_FUNCTION double Lav_getDoubleProperty(LavNode *node, unsigned int slot) {
	return 0.0;
}

LAV_PUBLIC_FUNCTION char* Lav_getStringProperty(LavNode* node, unsigned int slot) {
	return "";
}
