/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Demonstrates asynchronous audio output.*/
#include <libaudioverse/private_all.h>
#include <stdio.h>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	return;\
}\
} while(0)\

void main() {
	void* th;
	LavObject *graph;
	LavObject* node;
	LavError err = Lav_initializeLibrary();
	if(err != Lav_ERROR_NONE) {
		printf("Failed to initialize library. Error: %i", err);
		return;
	}
	Lav_createGraph(SR, 1024, &graph);
	Lav_createSineNode(graph, &node);
	Lav_graphSetOutputNode(graph, node);
	createAudioOutputThread(graph, 3, &th);
	Lav_setFloatProperty(node, Lav_SINE_FREQUENCY, 0);
	for(unsigned int i = 0; i < 1000; i++) {
		Lav_setFloatProperty(node, Lav_SINE_FREQUENCY, i);
		sleepFor(5);
	}
	stopAudioOutputThread(th);
	sleepFor(2000);
}
