/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <libaudioverse/private_structs3d.h>
#include <libaudioverse/libaudioverse3d.h>
#include <transmat.h>
#include <stdlib.h>
#include <libaudioverse/private_sources.h>

void worldListenerUpdateCallback(LavObject* obj, unsigned int slot);
void worldPreprocessingHook(LavObject* obj, void* param);

LavPropertyTableEntry worldPropertyTable[] = {
	{Lav_WORLD_LISTENER_ORIENTATION, Lav_PROPERTYTYPE_FLOAT6, "listener_orientation", {.f6val = {0, 0, -1, 0, 1, 0}}, worldListenerUpdateCallback},
	{Lav_WORLD_LISTENER_POSITION, Lav_PROPERTYTYPE_FLOAT3, "listener_position", {.f3val = {0, 0, 0}}, worldListenerUpdateCallback},
};

Lav_PUBLIC_FUNCTION LavError Lav_createWorld(LavObject**destination) {
	STANDARD_PREAMBLE;
	//we need a graph and a mixer node.  If we can't get them, just abort now and save ourselves trouble.
	LavObject *mixer, *graph;
	LavError err;
	//Todo: make these parameters configurable.
	err = Lav_createGraph(44100, 1024, &graph);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//todo: make these configurable too.
	Lav_createMixerNode(graph, 16, 2, &mixer);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//assign the preprocessing hook.
	((LavGraph*)graph)->preprocessing_hook = worldPreprocessingHook;
	LavObject* obj= calloc(1, sizeof(LavWorld));
	ERROR_IF_TRUE(obj == NULL, Lav_ERROR_MEMORY);
	err = initLavObject(0, 0,
	sizeof(worldPropertyTable)/sizeof(worldPropertyTable[0]), worldPropertyTable, Lav_OBJTYPE_WORLD, graph->block_size, graph->mutex, &obj);
	//we use the graph's mutex because we need to synchronize with it: we can't be touching properties while the graph is in use.
	//we use the graph's block size because it doesn't really matter right now but might later.
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//this is a convenience pointer wrapping the above pointer.
	LavWorld* const world = (LavWorld*)obj;

	world->mixer = mixer;
	world->graph = graph;
	err = Lav_graphSetOutputNode(graph, mixer);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);

	//make room for sources.
	//Todo: also make these hard-coded maxes configurable.
	LavSource* source_array = calloc(16, sizeof(LavSource*));
	ERROR_IF_TRUE(source_array == NULL, Lav_ERROR_MEMORY);
	world->sources = source_array;
	world->num_sources = 0;
	world->max_sources = 16;
	//we're going to use this world as the preprocessing hook's argument.
	((LavGraph*)world->graph)->preprocessing_hook_argument = world;
	*destination = obj;
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
void worldPreprocessingHook(LavObject* obj, void* param) {
	LavWorld *const world = (LavWorld*)param;
	for(unsigned int i = 0; i < world->num_sources; i++) {
		sourceUpdate(world->sources+i);
	}
}
