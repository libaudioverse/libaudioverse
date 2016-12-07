/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
	LavHandle server;
	ERRCHECK(Lav_initialize());
	ERRCHECK(Lav_createServer(44100, 1024, &server));
	ERRCHECK(Lav_serverSetOutputDevice(server, "default", 2));
	ERRCHECK(Lav_createBufferNode(server, &node));
	LavHandle buffer;
	ERRCHECK(Lav_createBuffer(server, &buffer));
	auto start = clock();
	ERRCHECK(Lav_bufferLoadFromFile(buffer, path));
	auto end = clock();
	auto diff = end-start;
	double durationMs = ((double)diff/CLOCKS_PER_SEC)*1000;
	printf("File loaded in %F ms\n", durationMs);
	ERRCHECK(Lav_nodeSetBufferProperty(node, Lav_BUFFER_BUFFER, buffer));
	LavHandle limit;
	ERRCHECK(Lav_createHardLimiterNode(server, 2, &limit));
	ERRCHECK(Lav_nodeConnect(node, 0, limit, 0));
	ERRCHECK(Lav_bufferNodeSetEndCallback(node, endOfBufferCallback, nullptr));
	ERRCHECK(Lav_nodeConnectServer(limit, 0));
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
