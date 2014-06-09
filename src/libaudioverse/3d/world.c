/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_all.h>
#include <libaudioverse/private_structs3d.h>
#include <libaudioverse/libaudioverse3d.h>
#include <transmat.h>
#include <stdlib.h>
LavPropertyTableEntry worldPropertyTable[10] = {0};

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
	LavObject* obj= calloc(1, sizeof(LavWorld));
	ERROR_IF_TRUE(obj == NULL, Lav_ERROR_MEMORY);
	err = Lav_createObject(0, 0,
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
	*destination = obj;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}
