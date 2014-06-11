/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <transmat.h>
#include <stdlib.h>

void worldListenerUpdateCallback(LavObject* obj, unsigned int slot);
void worldPreprocessingHook(LavDevice* obj, void* param);

LavPropertyTableEntry worldPropertyTable[] = {
	{Lav_WORLD_LISTENER_ORIENTATION, Lav_PROPERTYTYPE_FLOAT6, "listener_orientation", {.f6val = {0, 0, -1, 0, 1, 0}}, worldListenerUpdateCallback},
	{Lav_WORLD_LISTENER_POSITION, Lav_PROPERTYTYPE_FLOAT3, "listener_position", {.f3val = {0, 0, 0}}, worldListenerUpdateCallback},
};

Lav_PUBLIC_FUNCTION LavError Lav_createWorld(LavDevice* device, LavHrtfData *hrtf, LavObject**destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(device);
	CHECK_NOT_NULL(hrtf);
	CHECK_NOT_NULL(destination);
	LavObject *mixer;
	LavError err;
	//todo: make these configurable too.
	err = Lav_createMixerNode(device, 16, 2, &mixer);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//assign the preprocessing hook.
	device->preprocessing_hook = worldPreprocessingHook;
	LavObject* obj= calloc(1, sizeof(LavWorld));
	ERROR_IF_TRUE(obj == NULL, Lav_ERROR_MEMORY);
	err = initLavObject(0, 0,
	sizeof(worldPropertyTable)/sizeof(worldPropertyTable[0]), worldPropertyTable, Lav_OBJTYPE_WORLD, device, obj);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//this is a convenience pointer wrapping the above pointer.
	LavWorld* const world = (LavWorld*)obj;

	world->mixer = mixer;
	err = Lav_deviceSetOutputObject(device, mixer);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);

	//make room for sources.
	//Todo: also make these hard-coded maxes configurable.
	LavSource** source_array = calloc(16, sizeof(LavSource**));
	ERROR_IF_TRUE(source_array == NULL, Lav_ERROR_MEMORY);
	world->sources = source_array;
	world->num_sources = 0;
	world->max_sources = 16;
	//we're going to use this world as the preprocessing hook's argument.
	device->preprocessing_hook_argument = world;
	*destination = obj;
	//save the hrtf.
	world->hrtf = hrtf;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

//called when the listener properties are changed.
//updates the camera transform with whatever the new values are.
void worldListenerUpdateCallback(LavObject* obj, unsigned int slot) {
	TmVector at, up, pos;
	float* at_p = at.vec, *up_p =up.vec, *pos_p = pos.vec;
	Lav_getFloat6Property(obj, Lav_WORLD_LISTENER_ORIENTATION, at_p, at_p+1, at_p+2, up_p, up_p+1, up_p+2);
	Lav_getFloat3Property(obj, Lav_WORLD_LISTENER_POSITION, pos_p, pos_p+1, pos_p+2);
	at.vec[3] = 1;
	up.vec[3] = 1;
	pos.vec[3] = 1;
	Tm_cameraTransform(at, up, pos, &(((LavWorld*)obj)->camera_transform)); //we unfortunately do need all the parentheses.
}

//simply asks all sources to update themselves as they see fit.
void worldPreprocessingHook(LavDevice* device, void* param) {
	LavWorld *const world = (LavWorld*)param;
	for(unsigned int i = 0; i < world->num_sources; i++) {
		sourceUpdate(world->sources[i]);
	}
}

LavError worldAssociateSource(LavWorld* world, LavSource* source) {
	STANDARD_PREAMBLE;
	//find a free slot.
	unsigned int slot = 0, found = 0;
	for(unsigned int i = 0; i < world->max_sources; i++) {
		if(world->sources[i] == NULL) {
			slot = i;
			found = 1;
			break;
		}
	}
	if(found == 0) {
		SAFERETURN(Lav_ERROR_MEMORY);
	}
	//first, we put it in the slot, then hook it into the mixer.
	world->sources[slot] = source;
	LavError err = Lav_setParent((LavObject*)source, world->mixer, 0, world->base.device->channels*slot); //left stereo channel.
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	err = Lav_setParent((LavObject*)source, world->mixer, 1, world->base.device->channels*slot+1); //the right stereo channel.
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	source->world = world;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}
