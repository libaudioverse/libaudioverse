/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	return;\
}\
} while(0)\

void main(int argc, char** args) {
	if(argc != 2) {
		printf("Syntax: %s <path>");
		return;
	}

	char* path = args[1];
	LavObject *node;
	LavDevice* device;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createDefaultAudioOutputDevice(&device));
	ERRCHECK(Lav_createFileObject(device, path, &node));
	LavObject* atten, *limit, *mix;
	unsigned int fileChannels;
	ERRCHECK(Lav_objectGetOutputCount(node, &fileChannels));
	ERRCHECK(Lav_createAttenuatorObject(device, fileChannels, &atten));
	ERRCHECK(Lav_createHardLimiterObject(device, fileChannels == 1 ? 2 : fileChannels, &limit));
	ERRCHECK(Lav_createMixerObject(device, 1, fileChannels, &mix));
	for(unsigned int i = 0; i < fileChannels; i++) {
		ERRCHECK(Lav_objectSetParent(atten, i, node, i));
		ERRCHECK(Lav_objectSetParent(mix, i, atten, i));
		ERRCHECK(Lav_objectSetParent(limit, i, mix, i));
	}
	//this makes mono play through both channels.
	if(fileChannels == 1) {
		ERRCHECK(Lav_objectSetParent(mix, 1, atten, 0));
		ERRCHECK(Lav_objectSetParent(limit, 1, mix, 0));
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
			case 'p': Lav_objectSetFloatProperty(node, Lav_FILE_PITCH_BEND, value); break;
			case 'v': Lav_objectSetFloatProperty(atten, Lav_ATTENUATOR_MULTIPLIER, value); break;
			case 's': Lav_objectSetFloatProperty(node, Lav_FILE_POSITION, value); break;
			default: printf("Unrecognized command.\n"); break;
		}
	}
	return;
}
