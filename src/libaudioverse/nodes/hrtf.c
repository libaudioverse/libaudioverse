/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError hrtfProcessor(LavNode *node);

struct HrtfNodeData {
	float *history;
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
	STANDARD_PREAMBLE;
	LavError err = Lav_ERROR_NONE;
	CHECK_NOT_NULL(hrtf);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	LavNode* retval = NULL;
	err = Lav_createNode(1, 2, Lav_NODETYPE_HRTF, graph, &retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);

	retval->properties = makePropertyArrayFromTable(sizeof(hrtfPropertyTable)/sizeof(hrtfPropertyTable[0]), hrtfPropertyTable);
	retval->num_properties = sizeof(hrtfPropertyTable)/sizeof(hrtfPropertyTable[0]);

	ERROR_IF_TRUE(retval->properties == NULL, Lav_ERROR_MEMORY);

	retval->process = hrtfProcessor;

	HrtfNodeData *data = calloc(1, sizeof(HrtfNodeData));
	ERROR_IF_TRUE(data == NULL, Lav_ERROR_MEMORY);
	/*Making these one longer is intensional.  Sinze the reader is the same as the writer, and we never read backwards past hrir_length, this prevents us from seeing the most recent sample twice.*/
	float* history = calloc(hrtf->hrir_length+1, sizeof(float));
	ERROR_IF_TRUE(history == NULL , Lav_ERROR_MEMORY);
	data->history = history;
	data->hrir_length = hrtf->hrir_length;
	data->hrtf = hrtf;

	//make room for the hrir itself.
	data->left_response = calloc(hrtf->hrir_length, sizeof(float));
	data->right_response = calloc(hrtf->hrir_length, sizeof(float));
	ERROR_IF_TRUE(data->left_response == NULL || data->right_response == NULL, Lav_ERROR_MEMORY);
	retval->data = data;

	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError hrtfProcessor(LavNode *node) {
	HrtfNodeData *data = node->data;
	float azimuth, elevation;
	Lav_getFloatProperty(node, Lav_HRTF_AZIMUTH, &azimuth);
	Lav_getFloatProperty(node, Lav_HRTF_ELEVATION, &elevation);
	//compute hrirs.
	hrtfComputeCoefficients(data->hrtf, elevation, azimuth, data->left_response, data->right_response);
	//this is convolution, with a twist: it uses a ringbuffer to avoid a ton of unnecessary copies.
	for(unsigned int i = 0; i < node->graph->block_size; i++) {
		data->history_pos = ringmodi(data->history_pos+1, data->hrir_length); //putting it here for clarity. It doesn't matter if this is before everything or after it.
		//first thing we do: read a sample into the ringbuffer.
		*(data->history+data->history_pos) = node->inputs[0][i];
		float outLeft=0, outRight=0;
		for(unsigned int j = 0; j < data->hrir_length; j++) {
			//standard convolution.
			unsigned int index = ringmodi(data->history_pos-j, data->hrir_length);
			outLeft += data->left_response[j]*data->history[index];
			outRight += data->right_response[j]*data->history[index];
		}
		node->outputs[0][i] = outLeft;
		node->outputs[1][i] = outRight;
	}
	return Lav_ERROR_NONE;
}
