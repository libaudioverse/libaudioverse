/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>

Lav_PUBLIC_FUNCTION LavError Lav_createGraph(float sr, unsigned int blockSize, LavObject **destination) {
	STANDARD_PREAMBLE;
	ERROR_IF_TRUE(blockSize < 1 || blockSize > Lav_MAX_BLOCK_SIZE, Lav_ERROR_RANGE);
	LavGraph *retval = calloc(1, sizeof(LavGraph));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	void* mut;
	LavError err = createMutex(&mut);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	err = initLavObject(0, 0, Lav_OBJTYPE_GRAPH, blockSize, mut, (LavObject*)retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	retval->nodes = (LavObject**)calloc(8, sizeof(LavObject*));
	ERROR_IF_TRUE(retval->nodes == NULL, Lav_ERROR_MEMORY);
	retval->nodes_length = 8;
	retval->sr = sr;
	retval->block_size = blockSize;
	*destination = (LavObject*)retval;
	SAFERETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_graphGetOutputNode(LavObject *graph, LavObject **destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	*destination = ((LavGraph*)graph)->output_node;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_graphSetOutputNode(LavObject *graph, LavObject *node) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(node);
	LOCK(graph->mutex);
	((LavGraph*)graph)->output_node = node;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

//Internal function for associating nodes with graphs.
//If the node is already in the graph, do nothing; otherwise, do necessary reallocations.
void graphAssociateNode(LavObject *graph, LavObject*node) {
	//See if the node is in the graph.
	for(unsigned int i = 0; i < ((LavGraph*)graph)->node_count; i++) {
		if(((LavGraph*)graph)->nodes[i] == node) return;
	}

	if(((LavGraph*)graph)->node_count == ((LavGraph*)graph)->nodes_length) { //reallocation is needed.
		LavObject** new_array = (LavObject**)realloc(((LavGraph*)graph)->nodes, sizeof(LavObject**) * ((LavGraph*)graph)->nodes_length * 2);
		if(new_array == NULL) return;
		((LavGraph*)graph)->nodes = new_array;
		((LavGraph*)graph)->nodes_length = ((LavGraph*)graph)->nodes_length * 2;
	}

	//Otherwise, simply place it at node_count, and increment.
	*(((LavGraph*)graph)->nodes+((LavGraph*)graph)->node_count) = node;
	((LavGraph*)graph)->node_count += 1;
}

//exists so we can do a recursive call.
//Todo: abstract arrays of arbetrary items out completely.
struct AlreadySeenNodes {
	unsigned int count, max_length;
	LavObject** nodes;
};

Lav_PUBLIC_FUNCTION void graphProcessHelper(LavObject* node, struct AlreadySeenNodes *done, int recursingLevel) {
	if(recursingLevel == 0) {
		done = malloc(sizeof(struct AlreadySeenNodes));
		done->count = 0;
		done->nodes = calloc(16, sizeof(LavObject*));
		done->max_length = 16;
	}
	if(node == NULL) {
		return;
	}
	for(unsigned int i = 0; i < node->num_inputs; i++) {
		graphProcessHelper(node->input_descriptors[i].parent, done, recursingLevel+1);
	}
	for(unsigned int i = 0; i < done->count; i++) {
		if(done->nodes[i] == node) {
			return;
		}
	}
	if(done->count == done->max_length) {
		done->max_length *= 2;
		realloc(done->nodes, done->max_length*sizeof(LavNode*));
	}
	objectProcessSafe(node);
	done->nodes[done->count] = node;
	done->count += 1;
	if(recursingLevel == 0) {
		free(done->nodes);
		free(done);
	}
}

Lav_PUBLIC_FUNCTION LavError Lav_graphGetBlock(LavObject* obj, float* samples) {
	mutexLock(obj->mutex);
	graphProcessHelper(((LavGraph*)obj)->output_node, NULL, 0);
	Lav_objectReadBlock(((LavGraph*)obj)->output_node, samples);
	mutexUnlock(obj->mutex);
	return Lav_ERROR_NONE;
}
