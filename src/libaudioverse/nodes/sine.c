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

//the array must be one more than length.
float sineTable[44101];
unsigned int sineTableLength = 44100;
void* hasComputedSineTableFlag = NULL;

LavPropertyTableEntry sinePropertyTable[1] = {
	Lav_SINE_FREQUENCY, Lav_PROPERTYTYPE_FLOAT, "frequency", {.fval = 440},
};

void computeSineTable() {
	for(unsigned int i = 0; i < sineTableLength; i++) {
		sineTable[i] = sinf(2*PI*i/sineTableLength); //sine wave of frequency 1 for 1 second.
	}
	sineTable[sineTableLength] = sineTable[0]; //make sure the last sample mirrors the first.
}

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavGraph *graph, LavNode **destination) {
	WILL_RETURN(LavError);
	LavError err = Lav_ERROR_NONE;
	if(hasComputedSineTableFlag == NULL) err = createAFlag(&hasComputedSineTableFlag);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	ERROR_IF_TRUE(hasComputedSineTableFlag == NULL, Lav_ERROR_MEMORY);
	if(aFlagTestAndSet(hasComputedSineTableFlag) == 0) computeSineTable();

	CHECK_NOT_NULL(destination);
	CHECK_NOT_NULL(graph);
	LOCK(graph->mutex);
	LavNode *retval = NULL;
	err = Lav_createNode(0, 1, Lav_NODETYPE_SINE, graph, &retval);
	if(err != Lav_ERROR_NONE) RETURN(err);

	retval->properties = makePropertyArrayFromTable(sizeof(sinePropertyTable)/sizeof(sinePropertyTable[0]), sinePropertyTable);
	ERROR_IF_TRUE(retval->properties == NULL, Lav_ERROR_MEMORY);
	retval->num_properties = sizeof(sinePropertyTable)/sizeof(sinePropertyTable[0]);

	retval->process = sineProcessor;

	struct sineinfo* data = calloc(1, sizeof(struct sineinfo));
	ERROR_IF_TRUE(data == NULL, Lav_ERROR_MEMORY);

	data->table_delta = (float)sineTableLength/graph->sr;
	retval->data = data;

	*destination = retval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
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
