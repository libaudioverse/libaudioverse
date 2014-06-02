/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>
struct LavMixerData {
	float* accumulator_array;
	unsigned int inputs_per_parent;
};
typedef struct LavMixerData LavMixerData;

LavError mixerProcessor(LavNode *node);

Lav_PUBLIC_FUNCTION LavError Lav_createMixerNode(LavGraph *graph, unsigned int maxParents, unsigned int inputsPerParent, LavNode **destination) {
	STANDARD_PREAMBLE;
	LavError err = Lav_ERROR_NONE;
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	LavNode* retval = NULL;
	err = Lav_createNode(maxParents*inputsPerParent, inputsPerParent, Lav_NODETYPE_MIXER, graph, &retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);

	retval->num_properties = 0;

	retval->process = mixerProcessor;

	float *accum = calloc(inputsPerParent, sizeof(float));
	LavMixerData *data = calloc(0, sizeof(LavMixerData));
	ERROR_IF_TRUE(data == NULL || accum == NULL, Lav_ERROR_MEMORY);
	data->accumulator_array = accum;
	data->inputs_per_parent = inputsPerParent;
	retval->data = data;

	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError mixerProcessor(LavNode *node) {
	LavMixerData* data = node->data;
	for(unsigned int i = 0; i < node->graph->block_size; i++) {
		for(unsigned int j = 0; j < node->num_inputs; j++) {
			float samp = node->inputs[j][i];
			data->accumulator_array[j%data->inputs_per_parent] += samp;
		}
		for(unsigned int j = 0; j < node->num_outputs; j++) {
			node->outputs[j][i] = data->accumulator_array[j]/(node->num_inputs/data->inputs_per_parent);
			data->accumulator_array[j] = 0;
		}
	}
	return Lav_ERROR_NONE;
}
