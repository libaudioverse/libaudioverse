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
	if(argc != 2) {
		printf("Syntax: %s <path>");
		return 0;
	}

	char* path = args[1];
	LavObject *node;
	LavDevice* device;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createDefaultAudioOutputDevice(&device));
	ERRCHECK(Lav_createFileNode(device, path, &node));
	LavObject* atten, *limit, *mix;
	ERRCHECK(Lav_createAttenuatorNode(device, node->num_outputs, &atten));
	ERRCHECK(Lav_createHardLimiterNode(device, node->num_outputs == 1 ? 2 : node->num_outputs , &limit));
	ERRCHECK(Lav_createMixerNode(device, 1, 2, &mix));
	for(unsigned int i = 0; i < node->num_outputs; i++) {
		ERRCHECK(Lav_setParent(atten, node, i, i));
		ERRCHECK(Lav_setParent(mix, atten, i, i));
		ERRCHECK(Lav_setParent(limit, mix, i, i));
	}
	//this makes mono play through both channels.
	if(node->num_outputs == 1) {
		ERRCHECK(Lav_setParent(mix, atten, 0, 1));
		ERRCHECK(Lav_setParent(limit, mix, 0, 1));
	}
	ERRCHECK(Lav_deviceSetOutputObject(device, limit));

	//enter the transducer loop.
	char command[1024];
	printf("Commands: q(q)uit, (s)eek, (v)olume, (p)itch bend\n");
	while(1) {
		fgets(command, 1023, stdin);
		if(command[0] == 'q') break;
		float value;
		char* start = &command[1];
		while(*start == ' ') start+=1; //skip spaces.
		sscanf(start, "%f", &value);
		switch(command[0]) {
			case 'p': Lav_setFloatProperty(node, Lav_FILE_PITCH_BEND, value); break;
			case 'v': Lav_setFloatProperty(atten, Lav_ATTENUATOR_MULTIPLIER, value); break;
			case 's': Lav_setFloatProperty(node, Lav_FILE_POSITION, value); break;
			default: printf("Unrecognized command.\n"); break;
		}
	}
}
