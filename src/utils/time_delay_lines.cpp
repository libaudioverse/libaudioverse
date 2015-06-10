/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Given a file, produces 5 seconds of audio output at 44100 HZ.  Rather than playing it, however, this program prints how long it took and the estimated number that can be run in one thread in realtime.*/
#include "time_helper.hpp"
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#define NUM_LINES 250
#define NUM_TIMES 50
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
	LavHandle simulation;
	std::vector<LavHandle> lines;
	LavHandle sineObj;
	unsigned int numLines = 0;

	ERRCHECK(Lav_createSimulation(44100, BLOCK_SIZE, &simulation));
	ERRCHECK(Lav_createSineNode(simulation, &sineObj));

	lines.resize(NUM_LINES, 0);
	for(auto i = lines.begin(); i != lines.end(); i++) {
		LavHandle newObj;
		ERRCHECK(Lav_createCrossfadingDelayNode(simulation, 1.0, 1, &newObj));
		*i = newObj;
		ERRCHECK(Lav_nodeSetIntProperty(newObj, Lav_NODE_STATE, Lav_NODESTATE_ALWAYS_PLAYING));
		ERRCHECK(Lav_nodeSetFloatProperty(newObj, Lav_DELAY_DELAY, 0.5f));
		ERRCHECK(Lav_nodeConnect(sineObj, 0, newObj, 0));
	}
	float t= timeit([&] () {
		Lav_simulationGetBlock(simulation, 2, 1, storage);
	}, NUM_TIMES);
	Lav_shutdown();
	printf("Took %f seconds\n", t);
	//Compute estimate of how many per second.
	t/=NUM_LINES*NUM_TIMES;
	float blocksPerSec =44100/1024;
	float estimate = 1.0f/t/blocksPerSec;
	printf("Estimate: %f lines maximum\n", estimate);
}
