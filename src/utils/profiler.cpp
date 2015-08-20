/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/


#include "time_helper.hpp"
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <tuple>

#define BLOCK_SIZE 1024
#define SR 44100
#define ITERATIONS 50
float storage[BLOCK_SIZE*2] = {0};

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	Lav_shutdown();\
	exit(1);\
}\
} while(0)\

#define ENTRY(name, times, createLine) \
std::make_tuple(\
name,\
times,\
[] (LavHandle sim, int count) {\
LavHandle h;\
std::vector<LavHandle> v;\
for(int i=0; i < count; i++) {\
	ERRCHECK(createLine);\
	v.push_back(h);\
}\
return v;\
}\
)

LavError createCrossfader(LavHandle& sim, LavHandle& h) {
	ERRCHECK(Lav_createCrossfaderNode(sim, 2, 2, &h));
	ERRCHECK(Lav_crossfaderNodeCrossfade(h, 2.0, 1));
	return Lav_ERROR_NONE;
}

std::tuple<std::string, int, std::function<std::vector<LavHandle>(LavHandle, int)>> to_profile[] = {
ENTRY("sine", 1000, Lav_createSineNode(sim, &h)),
ENTRY("crossfading delay line", 1000, Lav_createCrossfadingDelayNode(sim, 0.1, 1, &h)),
ENTRY("biquad", 1000, Lav_createBiquadNode(sim, 1, &h)),
ENTRY("One-pole filter", 1000, Lav_createOnePoleFilterNode(sim, 1, &h)),
ENTRY("2-channel 2-input crossfader", 500, createCrossfader(sim, h)),
ENTRY("amplitude panner", 1000, Lav_createAmplitudePannerNode(sim, &h)),
ENTRY("HRTF panner", 30, Lav_createHrtfNode(sim, "default", &h)),
ENTRY("hard limiter", 1000, Lav_createHardLimiterNode(sim, 2, &h)),
ENTRY("channel splitter", 1000, Lav_createChannelSplitterNode(sim, 10, &h)),
ENTRY("channel merger", 1000, Lav_createChannelMergerNode(sim, 10, &h)),
ENTRY("noise", 100, Lav_createNoiseNode(sim, &h)),
ENTRY("square", 500, Lav_createSquareNode(sim, &h)),
ENTRY("ringmod", 1000, Lav_createRingmodNode(sim, &h)),
ENTRY("16x16 FDN", 1, Lav_createFeedbackDelayNetworkNode(sim, 1.0f, 16, &h)),
ENTRY("32x32 FDN", 1, Lav_createFeedbackDelayNetworkNode(sim, 1.0f, 32, &h)),
};
int to_profile_size=sizeof(to_profile)/sizeof(to_profile[0]);

void main(int argc, char** args) {
	int threads = 1;
	if(argc == 2) {
		sscanf(args[1], "%i", &threads);
		if(threads < 1) {
			printf("Threads must be greater 1.\n");
			return;
		}
	}
	printf("Running profile tests on %i threads\n", threads);
	ERRCHECK(Lav_initialize());
	for(int i = 0; i < to_profile_size; i++) {
		auto &info = to_profile[i];
		printf("Estimate for %s nodes: ", std::get<0>(info).c_str());
		LavHandle sim;
		ERRCHECK(Lav_createSimulation(SR, BLOCK_SIZE, &sim));
		ERRCHECK(Lav_simulationSetThreads(sim, threads));
		int times=std::get<1>(info);
		//If it's not at least threads, make it threads.
		if(times < threads) times = threads;
		auto handles=std::get<2>(info)(sim, times);
		for(auto h: handles) {
			ERRCHECK(Lav_nodeSetIntProperty(h, Lav_NODE_STATE, Lav_NODESTATE_ALWAYS_PLAYING));
		}
		float dur=timeit([&] () {
			ERRCHECK(Lav_simulationGetBlock(sim, 2, 1, storage));
		}, ITERATIONS);
		dur /= ITERATIONS;
		float estimate = (BLOCK_SIZE/(float)SR)/dur*times;
		printf("%f\n", estimate);
		for(auto h: handles) {
			ERRCHECK(Lav_handleDecRef(h));
		}
		ERRCHECK(Lav_handleDecRef(sim));
	}
}
