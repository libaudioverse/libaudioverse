/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>

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

Lav_PUBLIC_FUNCTION LavError Lav_createReadDevice(unsigned int blockSize, unsigned int channels, unsigned int sr, LavDevice** destination) {
	//simply delegate: this is a specific call to createGenericDevice.
	return createGenericDevice(blockSize, channels, sr, NULL, NULL, NULL, NULL, destination);
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceSetOutputObject(LavDevice* device, LavObject* object) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(device);
	CHECK_NOT_NULL(object);
	LOCK(device->mutex);
	ERROR_IF_TRUE(object->device != device, Lav_ERROR_CANNOT_CROSS_DEVICES);
	//Must have at least 1 output, or fail.
	ERROR_IF_TRUE(object->num_outputs == 0, Lav_ERROR_NO_OUTPUTS);
	device->output_object = object;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetBlock(LavDevice* device, float* destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(device);
	CHECK_NOT_NULL(destination);
	LOCK(device->mutex);
	SAFERETURN(device->get_block(device, destination));
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetOutputObject(LavDevice* device, LavObject** destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(device);
	CHECK_NOT_NULL(destination);
	LOCK(device->mutex);
	*destination = device->output_object;
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
		device->objects = new_object_array;
		device->max_object_count *= 2;
	}
	device->objects[device->object_count] = object;
	device->object_count += 1;
	object->device = device;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION void deviceProcessHelper(LavObject* object) {
	if(object == NULL || object->has_processed) { //either it's a null parent, or we've already processed it for someone else.
		return;
	}
	for(unsigned int i = 0; i < object->num_inputs; i++) {
		deviceProcessHelper(object->input_descriptors[i].parent);
	}
	if(object->process) { //protect against objects without processors, which can happen.
		//object->is_in_processor is the third parameter to property callbacks.
		object->is_in_processor = 1;
		object->process(object);
		object->is_in_processor = 0;
	}
	//mark the object as processed.
	object->has_processed = 1;
}

LavError deviceDefaultGetBlock(LavDevice* device, float* destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(device);
	CHECK_NOT_NULL(destination);
	//(maybe) run the preprocessing hook.
	if(device->preprocessing_hook) {
		device->preprocessing_hook(device, device->preprocessing_hook_argument);
	}
	if(device->output_object == NULL) {
		memset(destination, 0, sizeof(float)*device->channels*device->block_size);
	}
	else {
		deviceProcessHelper(device->output_object);
	}
	//handle the should_always_process.
	unsigned int did_process = 0;
	do {
		did_process = 0;
		for(unsigned int i = 0; i < device->object_count; i++) {
			if(device->objects[i]->should_always_process && device->objects[i]->has_processed == 0) {
				if(device->objects[i]->process) {
					device->objects[i]->is_in_processor = 1;
					device->objects[i]->process(device->objects[i]);
					device->objects[i]->is_in_processor = 0;
				}
				did_process = 1;
			}
		}
	} while(did_process);
	for(unsigned int i = 0; i < device->object_count; i++) {
		device->objects[i]->has_processed = 0;
	}
	for(unsigned int i = 0; i < device->block_size; i++) {
		for(unsigned int j = 0; j < device->channels; j++) {
			if(device->output_object == NULL||j >= device->output_object->num_outputs) { //either no output object, or not enough outputs on it.
				destination[i*device->channels+j] = 0;
				continue;
			}
			//read from the output, and copy it in.
			destination[i*device->channels+j] = device->output_object->outputs[j][i];
		}
	}
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