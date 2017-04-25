/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */


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
#define ITERATIONS 200
float storage[BLOCK_SIZE*2] = {0};
//This is a big block of data so we can profile buffers.
#define BUFFER_SIZE 32768*4
float buffer[BUFFER_SIZE] = {0.0f};

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
[] (LavHandle s, int count) {\
LavHandle h;\
std::vector<LavHandle> v;\
for(int i=0; i < count; i++) {\
	ERRCHECK(createLine);\
	v.push_back(h);\
}\
return v;\
}\
)

LavError createCrossfader(LavHandle& s, LavHandle& h) {
	ERRCHECK(Lav_createCrossfaderNode(s, 2, 2, &h));
	ERRCHECK(Lav_crossfaderNodeCrossfade(h, 2.0, 1));
	return Lav_ERROR_NONE;
}

LavError createBuffer(LavHandle s, LavHandle& h) {
	ERRCHECK(Lav_createBufferNode(s, &h));
	LavHandle b;
	ERRCHECK(Lav_createBuffer(s, &b));
	ERRCHECK(Lav_bufferLoadFromArray(b, 44100, 4, BUFFER_SIZE/4, buffer));
	ERRCHECK(Lav_nodeSetBufferProperty(h, Lav_BUFFER_BUFFER, b));
	return Lav_ERROR_NONE;
}

LavError createCrossfadingDelay(LavHandle s, LavHandle& h) {
	ERRCHECK(Lav_createCrossfadingDelayNode(s, 0.1, 1, &h));
	ERRCHECK(Lav_nodeSetFloatProperty(h, Lav_DELAY_DELAY, 0.1));
	return Lav_ERROR_NONE;
}

LavError createDoppleringDelay(LavHandle s, LavHandle& h) {
	ERRCHECK(Lav_createDoppleringDelayNode(s, 0.1, 1, &h));
	ERRCHECK(Lav_nodeSetFloatProperty(h, Lav_DELAY_DELAY, 0.1));
	return Lav_ERROR_NONE;
}

std::tuple<std::string, int, std::function<std::vector<LavHandle>(LavHandle, int)>> to_profile[] = {
ENTRY("sine", 1000, Lav_createSineNode(s, &h)),
ENTRY("Blit", 1000, Lav_createBlitNode(s, &h)),
ENTRY("4-channel buffer", 100, createBuffer(s, h)),
ENTRY("crossfading delay line", 1000, createCrossfadingDelay(s, h)),
ENTRY("Dopplering delay line", 1000, createDoppleringDelay(s, h)),
ENTRY("biquad", 1000, Lav_createBiquadNode(s, 1, &h)),
ENTRY("One-pole filter", 1000, Lav_createOnePoleFilterNode(s, 1, &h)),
ENTRY("2-channel 2-input crossfader", 500, createCrossfader(s, h)),
ENTRY("amplitude panner", 1000, Lav_createAmplitudePannerNode(s, &h)),
ENTRY("HRTF panner", 30, Lav_createHrtfNode(s, "default", &h)),
ENTRY("hard limiter", 1000, Lav_createHardLimiterNode(s, 2, &h)),
ENTRY("channel splitter", 1000, Lav_createChannelSplitterNode(s, 10, &h)),
ENTRY("channel merger", 1000, Lav_createChannelMergerNode(s, 10, &h)),
ENTRY("noise", 100, Lav_createNoiseNode(s, &h)),
ENTRY("ringmod", 1000, Lav_createRingmodNode(s, &h)),
ENTRY("16x16 FDN", 1, Lav_createFeedbackDelayNetworkNode(s, 1.0f, 16, &h)),
ENTRY("32x32 FDN", 1, Lav_createFeedbackDelayNetworkNode(s, 1.0f, 32, &h)),
};
int to_profile_size=sizeof(to_profile)/sizeof(to_profile[0]);

int main(int argc, char** args) {
	int threads = 1;
	if(argc == 2) {
		sscanf(args[1], "%i", &threads);
		if(threads < 1) {
			printf("Threads must be greater 1.\n");
			return 1;
		}
	}
	printf("Running profile tests on %i threads\n", threads);
	ERRCHECK(Lav_initialize());
	for(int i = 0; i < to_profile_size; i++) {
		auto &info = to_profile[i];
		printf("Estimate for %s nodes: ", std::get<0>(info).c_str());
		LavHandle s;
		ERRCHECK(Lav_createServer(SR, BLOCK_SIZE, &s));
		ERRCHECK(Lav_serverSetThreads(s, threads));
		int times=std::get<1>(info);
		//If it's not at least threads, make it threads.
		if(times < threads) times = threads;
		auto handles=std::get<2>(info)(s, times);
		for(auto h: handles) {
			ERRCHECK(Lav_nodeConnectServer(h, 0));
		}
		float dur=timeit([&] () {
			ERRCHECK(Lav_serverGetBlock(s, 2, 1, storage));
		}, ITERATIONS);
		dur /= ITERATIONS;
		float estimate = (BLOCK_SIZE/(float)SR)/dur*times;
		printf("%f\n", estimate);
		for(auto h: handles) {
			ERRCHECK(Lav_nodeDisconnect(h, 0, 0, 0));
			ERRCHECK(Lav_handleDecRef(h));
		}
		ERRCHECK(Lav_handleDecRef(s));
	}
}
