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
	Lav_createGraph(SR, &graph);
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
	createAudioOutputThread(graph, 1024, 3, &th);
	sleepFor(5000);
	stopAudioOutputThread(th);
}
