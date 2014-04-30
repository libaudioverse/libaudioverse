#include <stdlib.h>
#incllude <libaudioverse/libaudioverse.h>

LavNode *makeNode(unsigned int numInputs, unsigned int numOutputs, enum  Lav_NODETYPE type) {
	LavNode *retval = calloc(0, sizeof(LavNode));
	retval->num_inputs = numInputs;
	retval->num_outputs = numOutputs;
	retval->type = type;
	return retval;
}
