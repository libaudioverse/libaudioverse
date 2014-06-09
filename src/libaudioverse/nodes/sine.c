/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError sineProcessor(LavObject *obj);
struct sineinfo {
	float table_delta;
	unsigned int start;
	float offset;
};


LavPropertyTableEntry sinePropertyTable[1] = {
	{Lav_SINE_FREQUENCY, Lav_PROPERTYTYPE_FLOAT, "frequency", {.fval = 440}, NULL},
};

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavObject *graph, LavObject  **destination) {
	STANDARD_PREAMBLE;
	LavError err = Lav_ERROR_NONE;

	CHECK_NOT_NULL(destination);
	CHECK_NOT_NULL(graph);
	LOCK(graph->mutex);
	LavNode *retval = NULL;
	err = Lav_createNode(0, 1,
sizeof(sinePropertyTable)/sizeof(sinePropertyTable[0]), sinePropertyTable,
Lav_NODETYPE_SINE, graph, (LavObject**)&retval);
	if(err != Lav_ERROR_NONE) SAFERETURN(err);
	((LavObject*)retval)->process = sineProcessor;

	struct sineinfo* data = calloc(1, sizeof(struct sineinfo));
	ERROR_IF_TRUE(data == NULL, Lav_ERROR_MEMORY);

	data->table_delta = (float)sineTableLength/((LavGraph*)graph)->sr;
	retval->data = data;

	*destination = (LavObject*)retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError sineProcessor(LavObject *obj) {
	const LavNode* node = (LavNode*)obj;
	float freq = 0;
	float sr = node->graph->sr;
	Lav_getFloatProperty(obj, Lav_SINE_FREQUENCY, &freq);
	struct sineinfo *data = node->data;
	float delta = data->table_delta*freq;
	for(unsigned int i = 0; i < obj->block_size; i++) {
		float weight1 = 1-data->offset;
		float weight2 = data->offset;
		unsigned int samp1 = data->start;
		unsigned int samp2 = samp1+1;
		float sample = sineTable[samp1]*weight1+sineTable[samp2]*weight2;
		obj->outputs[0][i] = sample;
		data->offset += delta;
		while(data->offset >= 1) {
			data->start+=1;
			data->start %= sineTableLength;
			data->offset -= 1.0f;
		}
	}
	return Lav_ERROR_NONE;
}
