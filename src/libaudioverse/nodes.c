/**Handles functionality common to all nodes: linking, allocating, and freeing.*/
#include <stdlib.h>
#incllude <libaudioverse/libaudioverse.h>

void freeNode(LavNode *node) {
	free(node);
}

LavNode *makeNode(unsigned int size, unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type) {
	LavNode *retval = calloc(0, size);
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;
	retval->type = type;
	return retval;
}

LavNodeWithHistory *makeHistoryNode(unsigned int size, unsigned int numInputs,
	unsigned int numOutputs, enum Lav_NODETYPE type, unsigned int historyLength) {
	LavNodeWithHistory *retval = (LavNodeWithHistory*)makeNode(size, numInputs, numOutputs, type);
	retval->history_length = historyLength;
	retval->history = calloc(historyLength, sizeof(float));
	return retval;
}
