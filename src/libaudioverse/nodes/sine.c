#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError sineProcessor(LavNode *node, unsigned int count);

Lav_PUBLIC_FUNCTION LavError Lav_makeSineNode(LavNode **destination) {
	CHECK_NOT_NULL(destination);
	LavNode *retval = NULL;
	Lav_makeNode(sizeof(LavNode), 0, 1, 3, Lav_NODETYPE_SINE, &retval);
	retval->properties[Lav_SINE_FREQUENCY].type = Lav_PROPERTYTYPE_FLOAT;
	retval->properties[Lav_SINE_FREQUENCY].value.fval = 440.0;
	retval->properties[Lav_SINE_FREQUENCY].default_value.fval = 440.0;
	retval->properties[Lav_SINE_FREQUENCY].name = "frequency";
	retval->process = sineProcessor;
	*destination = retval;
	return Lav_ERROR_NONE;
}


LavError sineProcessor(LavNode *node, unsigned int count) {
	float freq;
	Lav_getFloatProperty(node, Lav_SINE_FREQUENCY, &freq);
	for(unsigned int i = 0; i < count; i++) {
		Lav_bufferWriteSample(node->outputs+i, sin(2*PI*freq*node->internal_time));
		node->internal_time += 1/node->sr;
	}
}
