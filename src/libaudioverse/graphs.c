#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include "graphs.h"
#include <SDL_mutex.h>

Lav_PUBLIC_FUNCTION LavError Lav_createGraph(float sr, LavGraph **destination) {
	LavGraph *retval = calloc(1, sizeof(LavGraph));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	retval->nodes = (LavNode**)calloc(8, sizeof(LavNode*));
	ERROR_IF_TRUE(retval->nodes == NULL, Lav_ERROR_MEMORY);
	retval->nodes_length = 8;

	retval->sr = sr;
	*destination = retval;

	//Let's get a mutex:
	retval->graph_lock = SDL_CreateMutex();
	ERROR_IF_TRUE(retval->graph_lock == NULL, Lav_ERROR_UNKNOWN);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_graphGetOutputNode(LavGraph *graph, LavNode **destination) {
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(destination);
	*destination = graph->output_node;
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_graphSetOutputNode(LavGraph *graph, LavNode *node) {
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(node);
	graph->output_node = node;
	return Lav_ERROR_NONE;
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

Lav_PUBLIC_FUNCTION Lav_graphReadAllOutputs(LavGraph *graph, unsigned int samples, float* destination) {
	return Lav_nodeReadAllOutputs(graph->output_node, samples, destination);
}
