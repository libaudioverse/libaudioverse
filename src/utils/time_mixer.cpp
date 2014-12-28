/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Given a file, produces 5 seconds of audio output at 44100 HZ.  Rather than playing it, however, this program prints how long it took and the estimated number that can be run in one thread in realtime.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#define SECONDS 5
#define BLOCK_SIZE 1024
float storage[BLOCK_SIZE*2] = {0};

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	Lav_shutdown();\
	return;\
}\
} while(0)\

void main(int argc, char** args) {
	ERRCHECK(Lav_initialize());
	LavSimulation* simulation;
	LavObject* mixer;
	LavObject* sineObj;
	unsigned int numInputs = 0;
	float timeDelta = 0.0f;

	ERRCHECK(Lav_createReadSimulation(44100, BLOCK_SIZE, &simulation));
	ERRCHECK(Lav_createSineObject(simulation, &sineObj));
	while(timeDelta < SECONDS) {
		numInputs += 10000;
		printf("Preparing to test with %u inputs...\n", numInputs);
		ERRCHECK(Lav_createMixerObject(simulation, numInputs, 1, &mixer));
		ERRCHECK(Lav_simulationSetOutputObject(simulation, mixer));
		for(int i = 0; i < numInputs; i++) {
			ERRCHECK(Lav_objectSetInput(mixer, i, sineObj, 0));
		}
		clock_t startTime = clock();
		for(unsigned int i = 0; i < SECONDS*44100; i+=BLOCK_SIZE) {
			Lav_simulationGetBlock(simulation, 2, 1, storage);
		}
		clock_t endTime = clock();
		timeDelta = (endTime-startTime)/(float)CLOCKS_PER_SEC;
		printf("Done.  Took %f seconds to process.\n", timeDelta);
		Lav_free(mixer);
	}
	Lav_shutdown();
}
