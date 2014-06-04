
/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**This example demonstrates how to perform synchronous audio using Portaudio and Libaudioverse, and serves as an example for how to get audio out of the library directly.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <portaudio.h>
#include <stdlib.h>
#include <stdio.h>

void main() {
	//First, make a bunch of objects, connect them together, and get 5 seconds of a sine wave.
	int sine_time = 5;
	float sr = 44100;
	LavGraph *graph;
	LavError err = Lav_initializeLibrary();
	if(err != Lav_ERROR_NONE) {
		printf("Failed to initialize library. Error: %i", err);
		return;
	}
	Lav_createGraph(sr, sr*sine_time, &graph); //graph with SINE_TIME blocksize.  Don't do this in real code, as it's memory hungry.
	LavNode *node;
	Lav_createSineNode(graph, &node);

	//Let's set its frequency to 150:
	Lav_setFloatProperty(node, Lav_SINE_FREQUENCY, 150.0);

	//Make it the output node:
	Lav_graphSetOutputNode(graph, node);

	//Get 5 seconds of audio.
	float *output = calloc((int)(sr*sine_time), sizeof(float));
	if(output == NULL) {
		printf("Error: could not allocate output array.");
		return;
	}

	Lav_graphReadAllOutputs(graph, output);

	//Finally, play it.
	Pa_Initialize();
	PaStream *stream;
	Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, sr, 128, NULL, NULL);
	Pa_StartStream(stream);
	Pa_WriteStream(stream, output, (int)(sr*sine_time));
	Pa_StopStream(stream);
	Pa_Terminate();
}
