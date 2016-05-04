/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	Lav_shutdown();\
	return 1;\
}\
} while(0)\

void endOfBufferCallback(LavHandle ignoredObject, void* ignored) {
	printf("End of file reached.\n");
}

int main(int argc, char** args) {
	if(argc != 2) {
		printf("Syntax: %s <path>", args[0]);
		return 1;
	}
	char* path = args[1];
	LavHandle node;
	LavHandle simulation;
	ERRCHECK(Lav_initialize());
	ERRCHECK(Lav_createSimulation(44100, 1024, &simulation));
	ERRCHECK(Lav_simulationSetOutputDevice(simulation, "default", 2));
	ERRCHECK(Lav_createBufferNode(simulation, &node));
	LavHandle buffer;
	ERRCHECK(Lav_createBuffer(simulation, &buffer));
	ERRCHECK(Lav_bufferLoadFromFile(buffer, path));
	ERRCHECK(Lav_nodeSetBufferProperty(node, Lav_BUFFER_BUFFER, buffer));
	LavHandle limit;
	ERRCHECK(Lav_createHardLimiterNode(simulation, 2, &limit));
	ERRCHECK(Lav_nodeConnect(node, 0, limit, 0));
	ERRCHECK(Lav_bufferNodeSetEndCallback(node, endOfBufferCallback, nullptr));
	ERRCHECK(Lav_nodeConnectSimulation(limit, 0));
	//enter the transducer loop.
	char command[1024];
	printf("Commands: q(q)uit, (s)eek, (v)olume, (p)itch bend\n");
	printf("pl(a)y/pause\n");
	bool isPlaying = true;
	while(1) {
		fgets(command, 1023, stdin);
		if(command[0] == 'q') break;
		float value;
		char* start = &command[1];
		while(*start == ' ') start+=1; //skip spaces.
		sscanf(start, "%f", &value);
		switch(command[0]) {
			case 'p': Lav_nodeSetDoubleProperty(node, Lav_BUFFER_RATE, value); break;
			case 'v': Lav_nodeSetFloatProperty(node, Lav_NODE_MUL, value); break;
			case 's': Lav_nodeSetDoubleProperty(node, Lav_BUFFER_POSITION, value); break;
			case 'a': isPlaying = ! isPlaying; Lav_nodeSetIntProperty(node, Lav_NODE_STATE, isPlaying == false ? Lav_NODESTATE_PAUSED: Lav_NODESTATE_PLAYING); break;
			default: printf("Unrecognized command.\n"); break;
		}
	}
	Lav_shutdown();
	return 0;
}
