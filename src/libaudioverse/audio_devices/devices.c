/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>

LavError deviceDefaultGetBlock(LavDevice* device, float* destination);
LavError deviceDefaultStart(LavDevice* device);
LavError deviceDefaultStop(LavDevice* device);
LavError deviceDefaultKill(LavDevice* device);

Lav_PUBLIC_FUNCTION LavError createGenericDevice(
	unsigned int blockSize,
	unsigned int channels,
	unsigned int sr,
	LavError (*getBlock)(LavDevice* device, float* destination),
	LavError (*start)(LavDevice* device),
	LavError (*stop)(LavDevice* device),
	LavError (*kill)(LavDevice* device),
	LavDevice** destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(destination);
	ERROR_IF_TRUE(blockSize == 0 || sr == 0 || channels == 0, Lav_ERROR_RANGE);
	//either we were provided an implementation, or we need the defaults.
	getBlock = getBlock ? getBlock : deviceDefaultGetBlock;
	kill = kill ? kill : deviceDefaultKill;
	start = start ? start : deviceDefaultStart;
	stop = stop ? stop : deviceDefaultStop;

	LavDevice* retval = calloc(1, sizeof(LavDevice));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	retval->start = start;
	retval->stop = stop;
	retval->kill = kill;
	retval->get_block = getBlock;

	void* mut;
	LavError err = createMutex(&mut);
	retval->mutex = mut;
	retval->sr = sr;
	retval->block_size = blockSize;
	retval->channels = channels;

	//we start with an allotment of 128 nodes, an overhead of less than 1kb, and suitable for something on the order of 128/4=32 3d sources.
	LavObject** object_array = calloc(128, sizeof(LavObject*));
	ERROR_IF_TRUE(object_array == NULL, Lav_ERROR_MEMORY);
	retval->objects = object_array;
	retval->object_count = 0;
	retval->max_object_count = 128;
	//that's it, return.
	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError deviceAssociateObject(LavDevice* device, LavObject* object) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(device);
	CHECK_NOT_NULL(object);
	//if we're already associated with this device, bail out.
	//we could check the array, but this is faster; and the association should never change outside this api.
	if(object->device == device) {
		SAFERETURN(Lav_ERROR_INTERNAL_BUG); //should never happen unless this function is called inappropriately.
	}
	if(object->device != NULL) { //this object already belongs to a device, error instead.
		SAFERETURN(Lav_ERROR_INTERNAL_BUG); //if this bubbles as far as the user, the api is broken.  The only way this can happen is if this function is called inappropriately.
	}
	if(device->object_count == device->max_object_count) {
		//simple power-of-2 expansion.
		LavObject** new_object_array = realloc(device->objects, device->max_object_count*2*sizeof(LavObject*));
		ERROR_IF_TRUE(new_object_array == NULL, Lav_ERROR_MEMORY);
		//we now set everything after the current max node position to NULL.
		for(unsigned int i = device->max_object_count; i < device->max_object_count*2; i++) {
			new_object_array[i] = NULL;
		}
		//and put it in.
		device->objects = new_object_array;
		device->max_object_count *= 2;
	}
	device->objects[device->object_count] = object;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError deviceDefaultGetBlock(LavDevice* device, float* destination) {
	STANDARD_PREAMBLE;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError deviceDefaultStart(LavDevice* device) {
	STANDARD_PREAMBLE;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError deviceDefaultStop(LavDevice* device) {
	STANDARD_PREAMBLE;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

LavError deviceDefaultKill(LavDevice* device) {
	STANDARD_PREAMBLE;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}