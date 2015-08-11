/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

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
	return;\
}\
} while(0)\

void main(int argc, char** args) {
	if(argc != 3) {
		printf("Usage:%s <soundfile> <hrtf>", args[0]);
		return;
	}
	char *soundFile = args[1], *hrtfFile = args[2];
	LavHandle simulation = 0;
	LavHandle node, world, source;
	ERRCHECK(Lav_initialize());
	ERRCHECK(Lav_createSimulation(44100, 1024, &simulation));
	ERRCHECK(Lav_simulationSetOutputDevice(simulation, -1, 2, 0.0, 0.1, 0.2));
	ERRCHECK(Lav_createEnvironmentNode(simulation, hrtfFile, &world));
	ERRCHECK(Lav_createBufferNode(simulation, &node));
	LavHandle buffer;
	ERRCHECK(Lav_createBuffer(simulation, &buffer));
	ERRCHECK(Lav_bufferLoadFromFile(buffer, soundFile));
	ERRCHECK(Lav_nodeSetBufferProperty(node, Lav_BUFFER_BUFFER, buffer));
	ERRCHECK(Lav_nodeSetIntProperty(world, Lav_ENVIRONMENT_DEFAULT_PANNER_STRATEGY, Lav_PANNING_STRATEGY_HRTF));
	ERRCHECK(Lav_createSourceNode(simulation, world, &source));
	ERRCHECK(Lav_nodeConnect(node, 0, source, 0));
	const int resolution = 1000, length = 3000; //length in ms.
	const float width = 30.0;
	ERRCHECK(Lav_nodeConnectSimulation(world, 0));
	//do a square over and over.
	while(1) {
		for(int i = 0; i < resolution; i++) {
			Lav_nodeSetFloat3Property(source, Lav_3D_POSITION, width*(i/(float)resolution)-width/2, 0, -width/2);
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_nodeSetFloat3Property(source, Lav_3D_POSITION, width/2, 0, (width*((float)i/resolution)-width/2) );
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_nodeSetFloat3Property(source, Lav_3D_POSITION, -(width*(i/(float)resolution)-width/2), 0, width/2);
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_nodeSetFloat3Property(source, Lav_3D_POSITION, -width/2, 0, -(width*((float)i/resolution)-width/2));
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
	}
	Lav_shutdown();
}
