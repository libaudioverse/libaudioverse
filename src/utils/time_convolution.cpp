/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

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
#define NUM_SOURCES 150
#define NUM_TIMES 200
float storage[BLOCK_SIZE*2] = {0};

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	Lav_shutdown();\
	return 1;\
}\
} while(0)\

int main(int argc, char** args) {
	ERRCHECK(Lav_initialize());
	if(argc < 2) {
		printf("Usage: %s <hrtf file> [thread_count]", args[0]);
		return 1;
	}
	LavHandle server;
	LavHandle world;
	std::vector<LavHandle> sources;
	LavHandle sineObj;
	unsigned int numSources = 0;
	int threads = 1;
	if(argc == 3) {
		sscanf(args[2], "%i", &threads);
		if(threads < 1) {
			printf("Threads must be greater than 0.\n");
			return 1;
		}
	}
	//some setup: create a world and a server.
	ERRCHECK(Lav_createServer(44100, BLOCK_SIZE, &server));
	ERRCHECK(Lav_serverSetThreads(server, threads));
	ERRCHECK(Lav_createEnvironmentNode(server, args[1], &world));
	ERRCHECK(Lav_nodeSetIntProperty(world, Lav_ENVIRONMENT_PANNING_STRATEGY, Lav_PANNING_STRATEGY_HRTF));
	ERRCHECK(Lav_createSineNode(server, &sineObj));
	ERRCHECK(Lav_nodeConnectServer(world, 0));
	printf("Running %u times with %u sources on %i threads\n", NUM_TIMES, NUM_SOURCES, threads);
	sources.resize(NUM_SOURCES, 0);
	//anywhere there's a null pointer, replace it with a source.
	for(auto i = sources.begin(); i != sources.end(); i++) {
		LavHandle newSource;
		ERRCHECK(Lav_createSourceNode(server, world, &newSource));
		ERRCHECK(Lav_nodeConnect(sineObj, 0, newSource, 0));
		*i = newSource;
	}
	float t = timeit([&] () {
		Lav_serverGetBlock(server, 2, 1, storage);
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
	return 0;
}
