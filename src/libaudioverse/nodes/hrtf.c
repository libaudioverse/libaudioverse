/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>
#include <string.h>

LavError hrtfProcessor(LavObject *obj);
//this is the processor we go to if the hrtf is moved.
LavError hrtfRecomputeHrirProcessor(LavObject *obj);
void hrtfPropertyChanged(LavObject* obj, int slot);

struct HrtfNodeData {
	float *history;
	float* left_response, *right_response;
	LavHrtfData *hrtf;
	unsigned int hrir_length, history_length;
};

typedef struct HrtfNodeData HrtfNodeData;

LavPropertyTableEntry hrtfPropertyTable[] = {
{Lav_HRTF_AZIMUTH, Lav_PROPERTYTYPE_FLOAT, "azimuth", {.fval = 0.0f}},
{Lav_HRTF_ELEVATION, Lav_PROPERTYTYPE_FLOAT, "elevation", {.fval = 0.0f}},
};

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfNode(LavObject *graph, LavHrtfData* hrtf, LavObject **destination) {
	STANDARD_PREAMBLE;
	LavError err = Lav_ERROR_NONE;
	CHECK_NOT_NULL(hrtf);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	LavNode* retval = NULL;
	err = Lav_createNode(1, 2,
sizeof(hrtfPropertyTable)/sizeof(hrtfPropertyTable[0]), hrtfPropertyTable,
Lav_NODETYPE_HRTF, graph, (LavObject**)&retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);

	((LavObject*)retval)->process = hrtfRecomputeHrirProcessor;

	//we now assign our callback to all properties.
	for(unsigned int i = 0; i < ((LavObject*)retval)->num_properties; i++) {
		((LavObject*)retval)->properties[i]->post_changed_callback = hrtfPropertyChanged;
	}
	HrtfNodeData *data = calloc(1, sizeof(HrtfNodeData));
	ERROR_IF_TRUE(data == NULL, Lav_ERROR_MEMORY);
	float* history = calloc(hrtf->hrir_length+retval->base.block_size, sizeof(float));
	ERROR_IF_TRUE(history == NULL , Lav_ERROR_MEMORY);
	data->history = history;
	data->hrir_length = hrtf->hrir_length;
	data->history_length = data->hrir_length+retval->base.block_size;
	data->hrtf = hrtf;

	//make room for the hrir itself.
	data->left_response = calloc(hrtf->hrir_length, sizeof(float));
	data->right_response = calloc(hrtf->hrir_length, sizeof(float));
	ERROR_IF_TRUE(data->left_response == NULL || data->right_response == NULL, Lav_ERROR_MEMORY);
	retval->data = data;

	*destination = (LavObject*)retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

void hrtfPropertyChanged(LavObject* obj, int slot) {
	obj->process = hrtfRecomputeHrirProcessor;
}

LavError hrtfRecomputeHrirProcessor(LavObject *obj) {
	const LavNode* node = (LavNode*)obj;
	HrtfNodeData *data = node->data;
	float azimuth, elevation;
	Lav_getFloatProperty((LavObject*)node, Lav_HRTF_AZIMUTH, &azimuth);
	Lav_getFloatProperty((LavObject*)node, Lav_HRTF_ELEVATION, &elevation);
	//compute hrirs.
	hrtfComputeCoefficients(data->hrtf, elevation, azimuth, data->left_response, data->right_response);
	obj->process = hrtfProcessor;
	return hrtfProcessor(obj);
}

LavError hrtfProcessor(LavObject* obj) {
	const LavNode* node = (LavNode*)obj;
	HrtfNodeData *data = node->data;
	//copy the last hrir_length samples to the beginning, copy the input to the end.
	//this moves the history back by a fixed amount.
	memcpy(data->history, data->history+data->history_length-data->hrir_length, data->hrir_length*sizeof(float)); //this does not overlap.
	memcpy(data->history+data->hrir_length, obj->inputs[0], obj->block_size*sizeof(float));
	for(unsigned int i = 0; i < obj->block_size; i++) {
		float outLeft=0, outRight=0;
		for(unsigned int j = 0; j < data->hrir_length; j++) {
			outLeft += data->left_response[j]*data->history[data->hrir_length+i-j];
			outRight += data->right_response[j]*data->history[data->hrir_length+i-j];
		}
		obj->outputs[0][i] = outLeft;
		obj->outputs[1][i] = outRight;
	}
	return Lav_ERROR_NONE;
}
