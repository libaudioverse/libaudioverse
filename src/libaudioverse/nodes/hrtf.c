/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError hrtfProcessor(LavNode *node, unsigned int count);

struct HrtfNodeData {
	float *left_history, *right_history;
	float* left_response, *right_response;
	LavHrtfData *hrtf;
	unsigned int hrir_length;
	unsigned int history_pos; //for the ringbuffers.
};

typedef struct HrtfNodeData HrtfNodeData;

LavPropertyTableEntry hrtfPropertyTable[] = {
{Lav_HRTF_AZIMUTH, Lav_PROPERTYTYPE_FLOAT, "azimuth", {.fval = 0.0f}},
{Lav_HRTF_ELEVATION, Lav_PROPERTYTYPE_FLOAT, "elevation", {.fval = 0.0f}},
};

Lav_PUBLIC_FUNCTION Lav_createHrtfNode(LavGraph *graph, LavHrtfData* hrtf, LavNode **destination) {
	WILL_RETURN(LavError);
	LavError err = Lav_ERROR_NONE;
	CHECK_NOT_NULL(hrtf);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	LavNode* retval = NULL;
	err = Lav_createNode(1, 2, Lav_NODETYPE_HRTF, graph, &retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);

	retval->properties = makePropertyArrayFromTable(sizeof(hrtfPropertyTable)/sizeof(hrtfPropertyTable[0]), hrtfPropertyTable);
	ERROR_IF_TRUE(retval->properties == NULL, Lav_ERROR_MEMORY);

	retval->process = hrtfProcessor;

	HrtfNodeData *data = calloc(1, sizeof(HrtfNodeData));
	ERROR_IF_TRUE(data == NULL, Lav_ERROR_MEMORY);
	float* leftHistory = calloc(hrtf->hrir_length, sizeof(float));
	float* rightHistory = calloc(hrtf->hrir_length, sizeof(float));
	ERROR_IF_TRUE(leftHistory == NULL || rightHistory == NULL, Lav_ERROR_MEMORY);
	data->left_history = leftHistory;
	data->right_history = rightHistory;
	data->hrir_length = hrtf->hrir_length;
	data->hrtf = hrtf;

	//make room for the hrir itself.
	data->left_response = calloc(hrtf->hrir_length, sizeof(float));
	data->right_response = calloc(hrtf->hrir_length, sizeof(float));
	ERROR_IF_TRUE(data->left_response == NULL || data->right_response == NULL, Lav_ERROR_MEMORY);
	retval->data = data;

	*destination = retval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

LavError hrtfProcessor(LavNode *node, unsigned int count) {
	HrtfNodeData *data = node->data;
	float azimuth, elevation;
	Lav_getFloatProperty(node, Lav_HRTF_AZIMUTH, &azimuth);
	Lav_getFloatProperty(node, Lav_HRTF_ELEVATION, &elevation);
	//compute hrirs.
	hrtfComputeCoefficients(data->hrtf, elevation, azimuth, data->left_response, data->right_response);

	//this is convolution, with a twist: it uses a ringbuffer to avoid a ton of unnecessary copies.
	for(unsigned int i = 0; i < count; i++) {
		data->history_pos = ringmod(data->history_pos+1, data->hrir_length); //putting it here for clarity. It doesn't matter if this is before everything or after it.
		//first thing we do: read a sample into the ringbuffer, duplicating onto both channels.
		Lav_streamReadSamples(node->inputs[0], 1, data->left_history+data->history_pos);
		//duplicate this to the right channel.
		data->right_history[data->history_pos] = data->left_history[data->history_pos];
		float outLeft=0, outRight=0;
		for(unsigned int j = 0; j < data->hrir_length; j++) {
			//standard convolution.
			unsigned int index = ringmod(data->history_pos-j, data->hrir_length);
			outLeft += data->left_response[j]*data->left_history[index];
			outRight += data->right_response[j]*data->right_history[index];
		}
		Lav_bufferWriteSample(node->outputs[0], outLeft);
		Lav_bufferWriteSample(node->outputs[1], outRight);
	}
	return Lav_ERROR_NONE;
}
