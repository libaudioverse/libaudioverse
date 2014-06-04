/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError attenuatorProcessor(LavNode *node);

LavPropertyTableEntry attenuatorPropertyTable[1] = {
	Lav_ATTENUATOR_MULTIPLIER, Lav_PROPERTYTYPE_FLOAT, "multiplier", {.fval = 440},
};

Lav_PUBLIC_FUNCTION LavError Lav_createAttenuatorNode(LavGraph *graph, unsigned int numInputs, LavNode **destination) {
	STANDARD_PREAMBLE;
	LavError err = Lav_ERROR_NONE;

	CHECK_NOT_NULL(destination);
	CHECK_NOT_NULL(graph);
	ERROR_IF_TRUE(numInputs <= 0, Lav_ERROR_RANGE);
	LOCK(graph->mutex);
	LavNode *retval = NULL;
	err = Lav_createNode(numInputs, numInputs, Lav_NODETYPE_ATTENUATOR, graph, &retval);
	if(err != Lav_ERROR_NONE) SAFERETURN(err);

	retval->properties = makePropertyArrayFromTable(sizeof(attenuatorPropertyTable)/sizeof(attenuatorPropertyTable[0]), attenuatorPropertyTable);
	ERROR_IF_TRUE(retval->properties == NULL, Lav_ERROR_MEMORY);
	retval->num_properties = sizeof(attenuatorPropertyTable)/sizeof(attenuatorPropertyTable[0]);

	retval->process = attenuatorProcessor;

	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError attenuatorProcessor(LavNode *node) {
	float mul = 0;
	float sr = node->graph->sr;
	Lav_getFloatProperty(node, Lav_ATTENUATOR_MULTIPLIER, &mul);
	float delta = data->table_delta*freq;
	for(unsigned int i = 0; i < node->graph->block_size; i++) {
		for9unsigned int o = 0; o < node->num_outputs; o++) {
			node->outputs[o][i] = node->inputs[o][i]*mul;
		}
	}
	return Lav_ERROR_NONE;
}
