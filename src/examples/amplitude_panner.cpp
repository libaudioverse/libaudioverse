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
	return;\
}\
} while(0)\

void main(int argc, char** args) {
	if(argc != 2) {
		printf("Usage: %s <sound file>", args[0]);
		return;
	}
	LavDevice* device;
	LavObject* fileNode, *panNode, *limit;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createDefaultAudioOutputDevice(&device));
	ERRCHECK(Lav_createFileObject(device, args[1], &fileNode));
	float map[] = {90.0f, 270.0f};
	ERRCHECK(Lav_createAmplitudePannerObject(device, 2, map, &panNode));

	ERRCHECK(Lav_objectSetParent(panNode, 0, fileNode, 0));
	ERRCHECK(Lav_createHardLimiterObject(device, 2, &limit));
	ERRCHECK(Lav_objectSetParent(limit, 0, panNode, 0));
	ERRCHECK(Lav_objectSetParent(limit, 1, panNode, 1));
	ERRCHECK(Lav_deviceSetOutputObject(device, limit));
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
			ERRCHECK(Lav_objectSetFloatProperty(panNode, Lav_PANNER_ELEVATION, elev));
			ERRCHECK(Lav_objectSetFloatProperty(panNode, Lav_PANNER_AZIMUTH, az));
			elevOrAz = 0;
			continue;
		}
	}
}
