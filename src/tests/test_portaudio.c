/**This is a test of portaudio which serves two purposes: offering a suitable debug case and demonstrating proper usage of the library.
This test does not involve anything else.*/
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>

void main() {
	if(Pa_Initialize() < 0) {
		printf("Error initializing portaudio.\n");
		return;
	}

	printf("Portaudio version: %s\n", Pa_GetVersionText());

	//Let's print info on all host APIs.
	PaHostApiIndex hostApiCount;
	hostApiCount=Pa_GetHostApiCount();
	printf("Host API count: %i\n", hostApiCount);
	PaHostApiIndex defaultHostApi = Pa_GetDefaultHostApi();
	for(PaHostApiIndex i = 0; i < hostApiCount; i++) {
		PaHostApiInfo *info = NULL;
		info = Pa_GetHostApiInfo(i);
		printf("Info for host API: %s with %d devices.\n", info->name, info->deviceCount);
		for(int j = 0; j < info->deviceCount; j++) {
			PaDeviceIndex ind = Pa_HostApiDeviceIndexToDeviceIndex(i, j);
			PaDeviceInfo *devInfo = Pa_GetDeviceInfo(ind);
			printf("  %s (input channels=%d, output channels=%d, sr=%d)\n",
			devInfo->name, devInfo->maxInputChannels, devInfo->maxOutputChannels, (int)devInfo->defaultSampleRate);
		}
	}
	Pa_Terminate();
}