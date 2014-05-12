/**Demonstrates asynchronous audio output.*/
#include <libaudioverse/private_all.h>

void main() {
	void* th;
	LavGraph *graph;
	LavNode* node;
	Lav_createGraph(SR, &graph);
	Lav_createSineNode(graph, &node);
	Lav_graphSetOutputNode(graph, node);
	createAudioOutputThread(graph, 1024, 3, &th);
	Lav_setFloatProperty(node, Lav_SINE_FREQUENCY, 0);
	for(unsigned int i = 0; i < 1000; i++) {
		Lav_setFloatProperty(node, Lav_SINE_FREQUENCY, i);
		sleepFor(5);
	}
	stopAudioOutputThread(th);
	sleepFor(2000);
}
