/**This is a test of portaudio which serves two purposes: offering a suitable debug case and demonstrating proper usage of the library.
This test does not involve anything else.*/
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float time = 0;
const float sr = 48000;
float pi = 3.14159f;
float freq = 440;
float timeDelta = 1/48000.0;

int callback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo,
	PaStreamCallbackFlags flags, void* userdata) {
	float* actualInput = (float*) input;
	float *actualOutput = (float*)output;
	for(unsigned int i = 0; i < 2*frameCount; i+=2) {
		float sample = (float)sin(2*pi*time*freq);
		actualOutput[i] = sample;
		actualOutput[i+1]=sample;
		time += timeDelta;
	}
	return paContinue;
}

void main() {
	if(Pa_Initialize() < 0) {
		printf("Error initializing portaudio.\n");
		return;
	}

	printf("Portaudio version: %s\n", Pa_GetVersionText());

	//Let's print info on all host APIs.
	PaHostApiIndex hostApiCount;
	hostApiCount=Pa_GetHostApiCount();
	PaDeviceIndex defaultIn = Pa_GetDefaultInputDevice(), defaultOut = Pa_GetDefaultOutputDevice();
	printf("Host API count: %i\n", hostApiCount);
	PaHostApiIndex defaultHostApi = Pa_GetDefaultHostApi();
	for(PaHostApiIndex i = 0; i < hostApiCount; i++) {
		PaHostApiInfo const *info = NULL;
		info = Pa_GetHostApiInfo(i);
		printf("Info for host API: %s with %d devices.\n", info->name, info->deviceCount);
		for(int j = 0; j < info->deviceCount; j++) {
			PaDeviceIndex ind = Pa_HostApiDeviceIndexToDeviceIndex(i, j);
			PaDeviceInfo const *devInfo = Pa_GetDeviceInfo(ind);
			printf("  %s (input channels=%d, output channels=%d, sr=%d)\n",
			devInfo->name, devInfo->maxInputChannels, devInfo->maxOutputChannels, (int)devInfo->defaultSampleRate);
			if(ind == defaultIn) printf("Above is default input.\n");
			if(ind == defaultOut) printf("Above is default output.\n");
		}
	}

	printf("Synthesizing sine wave for 5 seconds.\n");
	//Open a stream.
	PaStream *stream = NULL;
	PaStreamParameters outputStreamFormat = {.device = Pa_GetDefaultOutputDevice(), .channelCount = 2,
	.sampleFormat = paFloat32, .suggestedLatency = 0.001f, .hostApiSpecificStreamInfo = NULL};

	if(Pa_IsFormatSupported(NULL, &outputStreamFormat, sr) != paFormatIsSupported) {
		printf("Default format unsupported.\n");
		return;
	}

	PaError err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sr, 8192, callback, NULL);
	if(err <0) {
		printf("Error: %s\n", Pa_GetErrorText(err));
		return;
	}
	//print info on the stream.
	PaStreamInfo *streamInfo = Pa_GetStreamInfo(stream);
	printf("Stream sample rate: %f\n", streamInfo->sampleRate);
	Pa_StartStream(stream);
	Pa_Sleep(5000);
	Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();
}