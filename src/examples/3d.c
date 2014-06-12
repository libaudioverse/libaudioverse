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

void main(int argc, char** args) {
	if(argc != 3) {
		printf("Usage:%s <soundfile> <hrtf>", args[0]);
		return;
	}
	char *soundFile = args[1], *hrtfFile = args[2];
	LavDevice* device;
	LavObject* node, *world, *source;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createDefaultAudioOutputDevice(&device));
	LavHrtfData* hrtf;
	ERRCHECK(Lav_createHrtfData(hrtfFile, &hrtf));
	ERRCHECK(Lav_createWorld(device, hrtf, &world));
	ERRCHECK(Lav_createFileNode(device, soundFile, &node));
	ERRCHECK(Lav_createMonoSource(node, world, &source));
	for(int i = 0; i < 10; i++) {
		Lav_setFloat3Property(source, Lav_SOURCE_POSITION, i-5, 0, -1);
		sleepFor(500);
	}
	sleepFor(2000);
}
