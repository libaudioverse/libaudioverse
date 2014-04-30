#include <stdlib.h>
#incllude <libaudioverse/libaudioverse.h>

LavNode *makeNode(unsigned int size, unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type) {
	LavNode *retval = calloc(0, size);
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;
	retval->type = type;
	return retval;
}
