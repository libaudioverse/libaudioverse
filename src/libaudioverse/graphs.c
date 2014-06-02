/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include "graphs.h"

Lav_PUBLIC_FUNCTION LavError Lav_createGraph(float sr, unsigned int blockSize, LavGraph **destination) {
	WILL_RETURN(LavError);
	LavGraph *retval = calloc(1, sizeof(LavGraph));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	retval->nodes = (LavNode**)calloc(8, sizeof(LavNode*));
	ERROR_IF_TRUE(retval->nodes == NULL, Lav_ERROR_MEMORY);
	ERROR_IF_TRUE(blockSize < 1, Lav_ERROR_RANGE);
	retval->nodes_length = 8;

	retval->sr = sr;
	retval->block_size = blockSize;
	*destination = retval;

	//Let's get a mutex:
	createMutex(&(retval->mutex));
	ERROR_IF_TRUE(retval->mutex == NULL, Lav_ERROR_UNKNOWN);
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_graphGetOutputNode(LavGraph *graph, LavNode **destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	*destination = graph->output_node;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_graphSetOutputNode(LavGraph *graph, LavNode *node) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(node);
	LOCK(graph->mutex);
	graph->output_node = node;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

//Internal function for associating nodes with graphs.
//If the node is already in the graph, do nothing; otherwise, do necessary reallocations.
void graphAssociateNode(LavGraph *graph, LavNode *node) {
	//See if the node is in the graph.
	for(unsigned int i = 0; i < graph->node_count; i++) {
		if(graph->nodes[i] == node) return;
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
Lav_PUBLIC_FUNCTION void graphProcessHelper(LavNode* node) {
	if(node == NULL) {
		return;
	}
	for(unsigned int i = 0; i < node->num_inputs; i++) {
		graphProcessHelper(node->input_descriptors[i].parent);
	}
		node->process(node);
}


Lav_PUBLIC_FUNCTION Lav_graphReadAllOutputs(LavGraph *graph, float* destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(destination);
	LOCK(graph->mutex);
	graphProcessHelper(graph->output_node);	
	RETURN(Lav_nodeReadBlock(graph->output_node, destination));
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}
