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

#define BLOCK_SIZE 1024
#define NUM_SOURCES 250
#define NUM_TIMES 50
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
	if(argc < 2) {
		printf("Usage: %s <hrtf file> [thread_count]", args[0]);
		return;
	}
	LavHandle simulation;
	LavHandle world;
	std::vector<LavHandle> sources;
	LavHandle sineObj;
	unsigned int numSources = 0;
	int threads = 1;
	if(argc == 3) {
		sscanf(args[2], "%i", &threads);
		if(threads < 1) {
			printf("Threads must be greater than 0.\n");
			return;
		}
	}
	//some setup: create a world and a simulation.
	ERRCHECK(Lav_createSimulation(44100, BLOCK_SIZE, &simulation));
	ERRCHECK(Lav_simulationSetThreads(simulation, threads));
	ERRCHECK(Lav_createEnvironmentNode(simulation, args[1], &world));
	ERRCHECK(Lav_nodeSetIntProperty(world, Lav_ENVIRONMENT_DEFAULT_PANNER_STRATEGY, Lav_PANNING_STRATEGY_HRTF));
	ERRCHECK(Lav_createSineNode(simulation, &sineObj));
	ERRCHECK(Lav_nodeConnectSimulation(world, 0));
	printf("Running %u times with %u sources on %i threads\n", NUM_TIMES, NUM_SOURCES, threads);
	sources.resize(NUM_SOURCES, 0);
	//anywhere there's a null pointer, replace it with a source.
	for(auto i = sources.begin(); i != sources.end(); i++) {
		LavHandle newSource;
		ERRCHECK(Lav_createSourceNode(simulation, world, &newSource));
		ERRCHECK(Lav_nodeConnect(sineObj, 0, newSource, 0));
		*i = newSource;
	}
	float t = timeit([&] () {
		Lav_simulationGetBlock(simulation, 2, 1, storage);
	}, NUM_TIMES);
	for(auto i: sources) {
		ERRCHECK(Lav_handleDecRef(i));
	}
	Lav_shutdown();
	printf("Took %f seconds\n", t);
	//Compute estimate of how many per second.
	t/=NUM_SOURCES*NUM_TIMES;
	float blocksPerSec =44100/1024;
	float estimate = 1.0f/t/blocksPerSec;
	printf("Estimate: %f sources maximum\n", estimate);
}
