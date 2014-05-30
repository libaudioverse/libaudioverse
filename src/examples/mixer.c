/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Demonstrates the hrtf node.*/
#include <libaudioverse/private_all.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

LavNode* makeNode(LavGraph *graph, char* file) {
	LavNode *retval;
	LavError err = Lav_createFileNode(graph, file, &retval);
	if(err != Lav_ERROR_NONE) {
		printf("Error: %i", err);
		return NULL;
	}
	return retval;
}

void main(int argc, char** args) {
	if(argc < 2) {
		printf("Usage: %s soundfile1 soundfile2 soundfile3 ... soundfilen", args[0]);
		return;
	}

	void* th;
	LavGraph *graph;
	LavNode** nodes;
	Lav_createGraph(SR, &graph);
	nodes = calloc(argc-1, sizeof(LavNode*));
	for(int i = 0; i < argc-1; i++) {
		LavNode* n = makeNode(graph, args[i+1]);
		if(n == NULL) return; //makeNode prints errors already.
		nodes[i] = n;
	}
	unsigned int channels = nodes[0]->num_outputs;
	for(int i = 0; i < argc-1; i++) {
		if(nodes[i]->num_outputs != channels) {
			printf("All files must have the same channel count.");
			return;
		}
	}

	//so far, so good. Make a mixer.
	LavNode* mixer;
	LavError err;
	err = Lav_createMixerNode(graph, argc-1, channels, &mixer);

	for(int input = 0; input < mixer->num_inputs ; input++) {
		Lav_setParent(mixer, nodes[input/channels], input%channels, input);
	}

	if(err != Lav_ERROR_NONE) {
		printf("Error: %i", err);
		return;
	}

	Lav_graphSetOutputNode(graph, mixer);
	createAudioOutputThread(graph, 1024, 3, &th);
	int shouldContinue = 1;
	char command[512] = "";
	printf("Enter q to quit.");
	while(shouldContinue) {
		scanf("%c", command);
		if(command[0] == 'q') {
			shouldContinue = 0;
			continue;
		}
	}
	stopAudioOutputThread(th);
}
