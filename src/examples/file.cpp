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

void endOfFileCallback(LavObject* ignoredObject, void* ignored) {
	printf("End of file reached.\n");
}

void main(int argc, char** args) {
	if(argc != 2) {
		printf("Syntax: %s <path>");
		return;
	}

	char* path = args[1];
	LavObject *node;
	LavSimulation* simulation;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createSimulationForDevice(-1, 44100, 1024, 2, &simulation));
	ERRCHECK(Lav_createFileObject(simulation, path, &node));
	LavObject* atten, *limit, *mix;
	unsigned int fileChannels;
	ERRCHECK(Lav_objectGetOutputCount(node, &fileChannels));
	ERRCHECK(Lav_createAttenuatorObject(simulation, fileChannels == 1 ? 2 : fileChannels, &atten));
	ERRCHECK(Lav_createHardLimiterObject(simulation, fileChannels == 1 ? 2 : fileChannels, &limit));
	ERRCHECK(Lav_createMixerObject(simulation, 1, fileChannels == 1 ? 2 : fileChannels, &mix));
	for(unsigned int i = 0; i < fileChannels; i++) {
		ERRCHECK(Lav_objectSetInput(atten, i, node, i));
		ERRCHECK(Lav_objectSetInput(mix, i, atten, i));
		ERRCHECK(Lav_objectSetInput(limit, i, mix, i));
	}
	//this makes mono play through both channels.
	if(fileChannels == 1) {
		ERRCHECK(Lav_objectSetInput(mix, 1, atten, 0));
		ERRCHECK(Lav_objectSetInput(limit, 1, mix, 0));
	}
	ERRCHECK(Lav_objectSetCallback(node, Lav_FILE_END_CALLBACK, endOfFileCallback, nullptr));
	ERRCHECK(Lav_simulationSetOutputObject(simulation, limit));

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
			case 'p': Lav_objectSetFloatProperty(node, Lav_FILE_PITCH_BEND, value); break;
			case 'v': Lav_objectSetFloatProperty(atten, Lav_ATTENUATOR_MULTIPLIER, value); break;
			case 's': Lav_objectSetDoubleProperty(node, Lav_FILE_POSITION, value); break;
			case 'a': isPlaying = ! isPlaying; Lav_objectSetIntProperty(node, Lav_OBJECT_STATE, isPlaying == false ? Lav_OBJECT_STATE_PAUSED: Lav_OBJECT_STATE_PLAYING); break;
			default: printf("Unrecognized command.\n"); break;
		}
	}
	return;
}
