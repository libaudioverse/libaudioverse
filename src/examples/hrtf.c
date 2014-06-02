/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Demonstrates the hrtf node.*/
#include <libaudioverse/private_all.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main(int argc, char** args) {
	if(argc != 3) {
		printf("Usage: %s <sound file> <hrtf file>", args[0]);
		return;
	}

	void* th;
	LavGraph *graph;
	LavNode* fileNode, *hrtfNode;
	Lav_createGraph(SR, 1024, &graph);
	LavError err = Lav_createFileNode(graph, args[1], &fileNode);
	if(err != Lav_ERROR_NONE) {
		printf("Error: %d", err);
		return;
	}

	LavHrtfData *hrtf = NULL;
	err = Lav_createHrtfData(args[2], &hrtf);
	if(err != Lav_ERROR_NONE) {
		printf("Error: %d", err);
		return;
	}
	err = Lav_createHrtfNode(graph, hrtf, &hrtfNode);
	if(err != Lav_ERROR_NONE) {
		printf("Error: %d", err);
		return;
	}
	Lav_setParent(hrtfNode, fileNode, 0, 0);
	Lav_graphSetOutputNode(graph, hrtfNode);
	createAudioOutputThread(graph, 3, &th);
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
			Lav_setFloatProperty(hrtfNode, Lav_HRTF_ELEVATION, elev);
			Lav_setFloatProperty(hrtfNode, Lav_HRTF_AZIMUTH, az);
			elevOrAz = 0;
			continue;
		}
	}
	stopAudioOutputThread(th);
}
