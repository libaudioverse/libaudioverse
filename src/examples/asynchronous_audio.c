/**Demonstrates asynchronous audio output.*/
#include <libaudioverse/private_all.h>

void main() {
	void* th;
	LavGraph *graph;
	LavNode* node;
	Lav_createGraph(SR, &graph);
	Lav_createSineNode(graph, &node);
	Lav_graphSetOutputNode(graph, node);
	createAudioOutputThread(graph, 128, 2, &th);
	while(1) {
		sleepFor(100);
	}
}
