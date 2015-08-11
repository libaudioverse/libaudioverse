/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Demonstrates the hrtf node.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	Lav_shutdown();\
	return;\
}\
} while(0)\

void main(int argc, char** args) {
	if(argc != 3) {
		printf("Usage: %s <sound file> <channels>", args[0]);
		return;
	}
	int channels = 0;
	sscanf(args[2], "%d", &channels);
	if(channels != 2
&& channels != 6
&& channels != 8) {
		printf("Cannot work with %d channels", channels);
		return;
	}
	LavHandle simulation;
	LavHandle bufferNode, panNode, limit;
	ERRCHECK(Lav_initialize());
	ERRCHECK(Lav_createSimulation(44100, 1024, &simulation));
	ERRCHECK(Lav_simulationSetOutputDevice(simulation, -1, channels, 0.0, 0.1, 0.2));
	ERRCHECK(Lav_createBufferNode(simulation, &bufferNode));
	LavHandle buffer;
	ERRCHECK(Lav_createBuffer(simulation, &buffer));
	ERRCHECK(Lav_bufferLoadFromFile(buffer, args[1]));
	ERRCHECK(Lav_nodeSetBufferProperty(bufferNode, Lav_BUFFER_BUFFER, buffer));
	ERRCHECK(Lav_createMultipannerNode(simulation, "default", &panNode));
	int pantype =Lav_PANNING_STRATEGY_STEREO;
	switch(channels) {
		case 6: pantype =Lav_PANNING_STRATEGY_SURROUND51; break;
		case 8: pantype=Lav_PANNING_STRATEGY_SURROUND71; break;
	}
	ERRCHECK(Lav_nodeSetIntProperty(panNode, Lav_PANNER_STRATEGY, pantype));
	ERRCHECK(Lav_nodeConnect(bufferNode, 0, panNode, 0));
	ERRCHECK(Lav_createHardLimiterNode(simulation, channels, &limit));
	ERRCHECK(Lav_nodeConnect(panNode, 0, limit, 0));
	ERRCHECK(Lav_nodeConnectSimulation(limit, 0));
	int shouldContinue = 1;
	printf("Enter pairs of numbers separated by whitespace, where the first is azimuth (anything) and the second\n"
"is elevation (-90 to 90).\n"
"Input q to quit.\n");
	char command[512] = "";
	float elev = 0, az = 0;
	int elevOrAz = 0;
	while(shouldContinue) {
		scanf("%s", command);
		if(command[0] == 'q') {
			shouldContinue = 0;
			continue;
		}
		if(elevOrAz == 0) {
			sscanf(command, "%f", &az);
			elevOrAz = 1;
			continue;
		}
		else if(elevOrAz == 1) {
			sscanf(command, "%f", &elev);
			ERRCHECK(Lav_nodeSetFloatProperty(panNode, Lav_PANNER_ELEVATION, elev));
			ERRCHECK(Lav_nodeSetFloatProperty(panNode, Lav_PANNER_AZIMUTH, az));
			elevOrAz = 0;
			continue;
		}
	}
	Lav_shutdown();
}
