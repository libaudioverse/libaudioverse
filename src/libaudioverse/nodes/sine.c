/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError sineProcessor(LavNode *node);
struct sineinfo {
	float table_delta;
	unsigned int start;
	float offset;
};


LavPropertyTableEntry sinePropertyTable[1] = {
	Lav_SINE_FREQUENCY, Lav_PROPERTYTYPE_FLOAT, "frequency", {.fval = 440},
};

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavGraph *graph, LavNode **destination) {
	STANDARD_PREAMBLE;
	LavError err = Lav_ERROR_NONE;

	CHECK_NOT_NULL(destination);
	CHECK_NOT_NULL(graph);
	LOCK(graph->mutex);
	LavNode *retval = NULL;
	err = Lav_createNode(0, 1, Lav_NODETYPE_SINE, graph, &retval);
	if(err != Lav_ERROR_NONE) SAFERETURN(err);

	retval->properties = makePropertyArrayFromTable(sizeof(sinePropertyTable)/sizeof(sinePropertyTable[0]), sinePropertyTable);
	ERROR_IF_TRUE(retval->properties == NULL, Lav_ERROR_MEMORY);
	retval->num_properties = sizeof(sinePropertyTable)/sizeof(sinePropertyTable[0]);

	retval->process = sineProcessor;

	struct sineinfo* data = calloc(1, sizeof(struct sineinfo));
	ERROR_IF_TRUE(data == NULL, Lav_ERROR_MEMORY);

	data->table_delta = (float)sineTableLength/graph->sr;
	retval->data = data;

	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError sineProcessor(LavNode *node) {
	float freq = 0;
	float sr = node->graph->sr;
	Lav_getFloatProperty(node, Lav_SINE_FREQUENCY, &freq);
	struct sineinfo *data = node->data;
	float delta = data->table_delta*freq;
	for(unsigned int i = 0; i < node->graph->block_size; i++) {
		float weight1 = 1-data->offset;
		float weight2 = data->offset;
		unsigned int samp1 = data->start;
		unsigned int samp2 = samp1+1;
		float sample = sineTable[samp1]*weight1+sineTable[samp2]*weight2;
		node->outputs[0][i] = sample;
		data->offset += delta;
		while(data->offset >= 1) {
			data->start+=1;
			data->start %= sineTableLength;
			data->offset -= 1.0f;
		}
	}
	return Lav_ERROR_NONE;
}
