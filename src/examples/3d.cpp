/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

/**Demonstrates 3d api.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <stdio.h>
#include <thread>
#include <chrono>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	Lav_shutdown();\
	return 1;\
}\
} while(0)\

int main(int argc, char** args) {
	if(argc != 3) {
		printf("Usage:%s <soundfile> <hrtf>", args[0]);
		return 1;
	}
	char *soundFile = args[1], *hrtfFile = args[2];
	LavHandle server = 0;
	LavHandle node, world, source;
	ERRCHECK(Lav_initialize());
	ERRCHECK(Lav_createServer(44100, 1024, &server));
	ERRCHECK(Lav_serverSetOutputDevice(server, "default", 2));
	ERRCHECK(Lav_createEnvironmentNode(server, hrtfFile, &world));
	ERRCHECK(Lav_createBufferNode(server, &node));
	LavHandle buffer;
	ERRCHECK(Lav_createBuffer(server, &buffer));
	ERRCHECK(Lav_bufferLoadFromFile(buffer, soundFile));
	ERRCHECK(Lav_nodeSetBufferProperty(node, Lav_BUFFER_BUFFER, buffer));
	ERRCHECK(Lav_nodeSetIntProperty(world, Lav_ENVIRONMENT_PANNING_STRATEGY, Lav_PANNING_STRATEGY_HRTF));
	ERRCHECK(Lav_createSourceNode(server, world, &source));
	ERRCHECK(Lav_nodeConnect(node, 0, source, 0));
	const int resolution = 1000, length = 3000; //length in ms.
	const float width = 30.0;
	ERRCHECK(Lav_nodeConnectServer(world, 0));
	//do a square over and over.
	while(1) {
		for(int i = 0; i < resolution; i++) {
			Lav_nodeSetFloat3Property(source, Lav_SOURCE_POSITION, width*(i/(float)resolution)-width/2, 0, -width/2);
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_nodeSetFloat3Property(source, Lav_SOURCE_POSITION, width/2, 0, (width*((float)i/resolution)-width/2) );
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_nodeSetFloat3Property(source, Lav_SOURCE_POSITION, -(width*(i/(float)resolution)-width/2), 0, width/2);
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_nodeSetFloat3Property(source, Lav_SOURCE_POSITION, -width/2, 0, -(width*((float)i/resolution)-width/2));
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
	}
	Lav_shutdown();
	return 0;
}
