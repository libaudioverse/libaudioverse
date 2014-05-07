/**This example demonstrates how to perform synchronous audio using Portaudio and Libaudioverse, and serves as an example for how to get audio out of the library directly.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <portaudio.h>
#include <stdlib.h>
#include <stdio.h>

void main() {
	//First, make a bunch of objects, connect them together, and get 5 seconds of a sine wave.
	float sine_time = 5.0;
	float sr = 44100;
	LavGraph *graph;
	Lav_createGraph(sr, &graph);
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

	Lav_graphReadAllOutputs(graph, (int)(sr*sine_time), output);

	//Finally, play it.
	Pa_Initialize();
	PaStream *stream;
	Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, sr, 128, NULL, NULL);
	Pa_StartStream(stream);
	Pa_WriteStream(stream, output, (int)(sr*sine_time));
	Pa_StopStream(stream);
	Pa_Terminate();
}
