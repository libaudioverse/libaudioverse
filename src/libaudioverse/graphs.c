#include <libaudioverse/private_all.h>
#include <stdlib.h>

Lav_PUBLIC_FUNCTION LavError Lav_createGraph(LavGraph **destination) {
	LavGraph *retval = calloc(1, sizeof(LavGraph));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	*destination = retval;
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
