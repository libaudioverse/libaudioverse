/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/private_all.h>

LavError hardLimiterProcessor(LavObject *node);

Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterNode(LavDevice* device, unsigned int numInputs, LavObject **destination) {
	STANDARD_PREAMBLE;
	LavError err = Lav_ERROR_NONE;
	CHECK_NOT_NULL(destination);
	CHECK_NOT_NULL(device);
	ERROR_IF_TRUE(numInputs <= 0, Lav_ERROR_RANGE);
	LOCK(device->mutex);
	LavNode *retval = NULL;
	err = Lav_createNode(numInputs, numInputs, 
0, 0,
Lav_NODETYPE_HARD_LIMITER, device, (LavObject**)&retval);
	if(err != Lav_ERROR_NONE) SAFERETURN(err);

	((LavObject*)retval)->process = hardLimiterProcessor;
	*destination = (LavObject*)retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError hardLimiterProcessor(LavObject *obj) {
	for(unsigned int i = 0; i < obj->device->block_size; i++) {
		for(unsigned int o = 0; o < obj->num_outputs; o++) {
			if(obj->inputs[o][i] > 1.0f) {
				obj->outputs[o][i] = 1.0f;
				continue;
			}
			else if(obj->inputs[o][i] < -1.0f) {
				obj->outputs[o][i] = -1.0f;
				continue;
			}
			obj->outputs[o][i] = obj->inputs[o][i];
		}
	}
	return Lav_ERROR_NONE;
}
