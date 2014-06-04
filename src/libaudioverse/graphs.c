/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include "graphs.h"

Lav_PUBLIC_FUNCTION LavError Lav_createGraph(float sr, unsigned int blockSize, LavGraph **destination) {
	STANDARD_PREAMBLE;
	ERROR_IF_TRUE(blockSize < 1 || blockSize > Lav_MAX_BLOCK_SIZE, Lav_ERROR_RANGE);
	LavGraph *retval = calloc(1, sizeof(LavGraph));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	void* mut;
	lavError err = createMutex(&mut);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	initLavObject(0, 0, lav_OBJTYPE_GRAPH, blockSize, mut, graph);
	retval->nodes = (LavObject**)calloc(8, sizeof(LavObject*));
	ERROR_IF_TRUE(retval->nodes == NULL, Lav_ERROR_MEMORY);
	retval->nodes_length = 8;
	retval->sr = sr;
	retval->block_size = blockSize;
	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_graphGetOutputNode(LavObject *graph, LavObject **destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	*destination = graph->output_node;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_graphSetOutputNode(LavObject *graph, LavObject *node) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(node);
	LOCK(graph->mutex);
	graph->output_node = node;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

//Internal function for associating nodes with graphs.
//If the node is already in the graph, do nothing; otherwise, do necessary reallocations.
void graphAssociateNode(LavNode *graph, LavNode*node) {
	//See if the node is in the graph.
	for(unsigned int i = 0; i < graph->node_count; i++) {
		if(graph->nodes[i] == (LavObject*)node) return;
	}

	if(graph->node_count == graph->nodes_length) { //reallocation is needed.
		LavNode** new_array = (LavNode**)realloc(graph->nodes, sizeof(LavNode**) * graph->nodes_length * 2);
		if(new_array == NULL) return;
		graph->nodes = new_array;
		graph->nodes_length = graph->nodes_length * 2;
	}

	//Otherwise, simply place it at node_count, and increment.
	*(graph->nodes+graph->node_count) = node;
	graph->node_count += 1;
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
