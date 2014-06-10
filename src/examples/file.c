/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_all.h> //our cross-platform sleep.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	return;\
}\
} while(0)\

int main(int argc, char** args) {
	if(argc != 4) {
		printf("Syntax: %s <path> <pitch_bend> <volume>", args[0]);
		return 0;
	}

	char* path = args[1];
	LavObject *node;
	LavObject *graph;
	LavError err = Lav_initializeLibrary();
	if(err != Lav_ERROR_NONE) {
		printf("Failed to initialize library. Error: %i", err);
		return;
	}
	Lav_createGraph(44100, 1024, &graph);
	err = Lav_createFileNode(graph, path, &node);
	if(err != Lav_ERROR_NONE) {
		printf("Error: %d", err);
		return 1;
	}

	float pitch_bend = 1.0f;
	sscanf(args[2], "%f", &pitch_bend);
	if(pitch_bend <= 0.0f) {
		pitch_bend = 1.0f;
	}
	float volume;
	sscanf(args[3], "%f", &volume);
	LavObject* atten;
	Lav_createAttenuatorNode(graph, node->num_outputs, &atten);
	for(unsigned int i = 0; i < node->num_outputs; i++) {
		Lav_setParent(atten, node, i, i);
	}

	Lav_setFloatProperty(node, Lav_FILE_PITCH_BEND, pitch_bend);
	Lav_setFloatProperty(atten, Lav_ATTENUATOR_MULTIPLIER, volume);

	Lav_graphSetOutputNode(graph, atten);
	void* th;
	createAudioOutputThread(graph, 3, &th);
	char pause[100];
	fgets(pause, 100, stdin);
	stopAudioOutputThread(th);
	return 0;
}
