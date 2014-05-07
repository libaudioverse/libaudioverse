#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError sineProcessor(LavNode *node, unsigned int count);

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavGraph *graph, LavNode **destination) {
	CHECK_NOT_NULL(destination);
	LavNode *retval = NULL;
	LavError err = Lav_createNode(0, 1, 3, Lav_NODETYPE_SINE, graph, &retval);
	if(err != Lav_ERROR_NONE) return err;
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
	float sr = node->graph->sr;
	Lav_getFloatProperty(node, Lav_SINE_FREQUENCY, &freq);
	for(unsigned int i = 0; i < count; i++) {
		Lav_bufferWriteSample(node->outputs+i, (float)sin(2*PI*freq*node->internal_time));
		node->internal_time += 1.0/sr;
	}
	return Lav_ERROR_NONE;
}
