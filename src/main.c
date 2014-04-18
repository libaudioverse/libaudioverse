#include "sndfile.h"
#include <stdio.h>
#include <stdlib.h>
#include "convolution_kernel.h"
#include <omp.h>

void main() {
	SF_INFO inputFileInfo, impulseResponseInfo, outputFileInfo;
	float* input, *leftImpulseResponse, *rightImpulseResponse;
	SNDFILE *inputFile, *outputFile, *impulseResponseFile;
	printf("Opening files.\n");
	inputFile = sf_open("sound.wav", SFM_READ, &inputFileInfo);
	outputFileInfo.channels = 2;
	outputFileInfo.samplerate = 44100;
	outputFileInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	outputFile = sf_open("out.wav", SFM_WRITE, &outputFileInfo);
	impulseResponseFile = sf_open("ir.wav", SFM_READ, &impulseResponseInfo);
	if(inputFile == 0 || outputFile == 0 || impulseResponseFile == 0) {
		printf("Error: could not open needed files.\n");
		return;
	}
	if(impulseResponseInfo.channels != 2) {
		printf("Error: impulse response is mono and not valid for 3d positioning.\n");
		return;
	}
	
	//here's the fun.
	printf("Reading files.\n");
	float *stereoResponse; //the impulse response, before we do evil things to it.
	stereoResponse = malloc(sizeof(float)*impulseResponseInfo.frames*impulseResponseInfo.channels); //get some memory.
	sf_count_t read = 0;
	while(read < impulseResponseInfo.frames) read += sf_readf_float(impulseResponseFile, &stereoResponse[read*2], impulseResponseInfo.frames);
	
	//get memory for the left and right response.
	leftImpulseResponse = malloc(sizeof(float)*impulseResponseInfo.frames);
	rightImpulseResponse = malloc(sizeof(float)*impulseResponseInfo.frames);
	
	//we walk every other sample of the array, first 0, 2, 4, 6, 8... and then 1, 3, 5, 7, 9...and copy it into the appropriate channel.
	float *channels[2] = {leftImpulseResponse, rightImpulseResponse};
	for(unsigned int offset = 0; offset < 2; offset++) {
		for(unsigned int counter = 0; counter < impulseResponseInfo.frames; counter++) {
			channels[offset][counter] = stereoResponse[offset+counter*2];
		}
	}
	
	//we have the impulse response separated out.  Let's do something with it.
	//we know this is a mono file, and that we must left-initialize it with 0. This gives:
	input = calloc(impulseResponseInfo.frames+inputFileInfo.frames, sizeof(float));
	float* startWriteAt = &input[impulseResponseInfo.frames]; //get a pointer past the beginning.
	
	read = 0;
	while(read < inputFileInfo.frames) read += sf_readf_float(inputFile, startWriteAt+read, inputFileInfo.frames);
	
	//normalize, to be safe.
	//Note that a single signal above 1.0 in the input can have an extremely adverse effect on the output, because it actually affects the next 128 samples.
	float maxval = 0;
	for(unsigned int i = 0; i < inputFileInfo.frames; i++) if(startWriteAt[i] > maxval) maxval = startWriteAt[i];
	for(unsigned int i = 0; i < inputFileInfo.frames; i++) startWriteAt[i]/=maxval;
	//We have everything we need to perform the two necessary convolutions, and the convolution kernel can handle the rest.
	float *leftChannel, *rightChannel;
	leftChannel = calloc(inputFileInfo.frames, sizeof(float));
	rightChannel = calloc(inputFileInfo.frames, sizeof(float));
	float* postClickRightChannel = calloc(inputFileInfo.frames, sizeof(float));
	float* postClickLeftChannel = calloc(inputFileInfo.frames, sizeof(float));
	printf("Beginning convolution.\n");
	float startTime = omp_get_wtime();
	convolution_kernel(leftChannel, inputFileInfo.frames, input, impulseResponseInfo.frames, leftImpulseResponse);
	convolution_kernel(rightChannel, inputFileInfo.frames, input, impulseResponseInfo.frames, rightImpulseResponse);
	float clickFilter[5] = {0.2, 0.2, 0.2, 0.2, -0.2}; //Important: this is lowpass. It gets rid of clicks, but distorts audio, especially if it has high frequencies.
	convolution_kernel(postClickLeftChannel, inputFileInfo.frames, leftChannel, 5, clickFilter);
	convolution_kernel(postClickRightChannel, inputFileInfo.frames, rightChannel, 5, clickFilter);
	leftChannel = postClickLeftChannel;
	rightChannel = postClickRightChannel;
	printf("Convolution complete.  Elapsed time is %f\n", omp_get_wtime()-startTime);
	printf("Writing output files.\n");
	//we have to put this back together now.  This is just as evil as before.
	float* output = calloc(inputFileInfo.frames*2, sizeof(float));
	for(unsigned int counter = 0; counter < inputFileInfo.frames*2; counter+=2) {
		output[counter] = leftChannel[counter/2];
		output[counter+1] = rightChannel[counter/2];
	}
	
	//write this monstrocity back to disk.
	sf_count_t write  = 0;
	while(write < inputFileInfo.frames*2) write+= sf_write_float(outputFile, output+write, 2*inputFileInfo.frames);
	sf_close(impulseResponseFile);
	sf_close(inputFile);
	sf_close(outputFile);
}
