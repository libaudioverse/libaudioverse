/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Demonstrates the hrtf node.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	return;\
}\
} while(0)\

LavObject* makeNode(LavDevice* device, char* file) {
	LavError err = Lav_ERROR_NONE;
	LavObject *retval;
	err = Lav_createFileObject(device, file, &retval);
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

	LavDevice* device;
	LavObject** nodes;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createDeviceForPhysicalOutput(-1, 44100, 1024, 2, &device));
	nodes = new LavObject*[argc-1];
	for(int i = 0; i < argc-1; i++) {
		LavObject* n = makeNode(device, args[i+1]);
		if(n == NULL) return; //makeNode prints errors already.
		nodes[i] = n;
	}
	unsigned int channels;
	Lav_objectGetOutputCount(nodes[0], &channels);
	for(int i = 0; i < argc-1; i++) {
		unsigned int outCount;
		Lav_objectGetOutputCount(nodes[i], &outCount);
		if(outCount != channels) {
			printf("All files must have the same channel count.");
			return;
		}
	}

	//so far, so good. Make a mixer.
	LavObject* mixer, *limit;
	ERRCHECK(Lav_createMixerObject(device, argc-1, channels, &mixer));
	ERRCHECK(Lav_createHardLimiterObject(device, channels, &limit));
	unsigned int mixInputCount;
	ERRCHECK(Lav_objectGetInputCount(mixer, &mixInputCount));
	for(unsigned int input = 0; input < mixInputCount; input++) {
		ERRCHECK(Lav_objectSetInput(mixer, input, nodes[input/channels], input%channels));
	}
	for(unsigned int i = 0; i < channels; i++) {
		ERRCHECK(Lav_objectSetInput(limit, i, mixer, i));
	}
	ERRCHECK(Lav_deviceSetOutputObject(device, limit));
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
}
