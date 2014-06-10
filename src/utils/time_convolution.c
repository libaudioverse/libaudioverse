/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Given a file, produces 5 seconds of audio output at 44100 HZ.  Rather than playing it, however, this program prints how long it took and the estimated number that can be run in one thread in realtime.*/
#include <libaudioverse/libaudioverse.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define seconds 5
#define blocksize 100
float storage[blocksize*2] = {0};

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	return;\
}\
} while(0)\

void main(int argc, char** args) {
	if(argc != 3) {
		printf("Usage: %s <sound file> <hrtf file>", args[0]);
		return;
	}

	LavDevice* device;
	LavObject* fileNode, *hrtfNode;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createReadDevice(blocksize, 2, 44100, &device));
	ERRCHECK(Lav_createFileNode(device, args[1], &fileNode));
	LavHrtfData *hrtf = NULL;
	ERRCHECK(Lav_createHrtfData(args[2], &hrtf));
	ERRCHECK(Lav_createHrtfNode(device, hrtf, &hrtfNode));
	ERRCHECK(Lav_setParent(hrtfNode, fileNode, 0, 0));
	ERRCHECK(Lav_deviceSetOutputObject(device, hrtfNode));

	printf("Convolving...\n");
	clock_t start;
	start = clock();
	for(unsigned int i = 0; i < 44100*seconds; i+= blocksize) {
		ERRCHECK(Lav_deviceGetBlock(device, storage));
	}
	clock_t dur = clock()-start;
	float secs = dur/(float)CLOCKS_PER_SEC;
	printf("Done.\n");
	printf("Convolution took %f seconds.\n", secs);
	printf("Total number of sources possible in realtime: %f", (float)seconds/secs);
}
