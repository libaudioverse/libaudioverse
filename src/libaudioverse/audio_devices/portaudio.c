/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	LavDevice *device;
	LavCrossThreadRingBuffer *ring_buffer;
	PaStream *stream;
	unsigned int block_size, mix_ahead, channels;
	void* running_flag; //when cleared, the thread dies.
} ThreadParams;

int audioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);
void audioOutputThread(void* vparam);

Lav_PUBLIC_FUNCTION LavError initializeAudioBackend() {
	PaError err = Pa_Initialize();
	if(err < 0) {
		return Lav_ERROR_CANNOT_INIT_AUDIO;
	}
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError createAudioOutputThread(LavDevice* device, unsigned int blockSize, unsigned int channels, unsigned int sr, unsigned int mixahead, void **destination) {
	STANDARD_PREAMBLE;
	mixahead+=1; //so we can actually mixahead 0 times.
	ThreadParams *param = calloc(1, sizeof(ThreadParams));
	LavError err;
	err = createCrossThreadRingBuffer(blockSize*mixahead, sizeof(float)*channels, &param->ring_buffer);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	param->block_size = blockSize;
	param->mix_ahead = mixahead;

	PaStream *stream;
	Pa_OpenDefaultStream(&stream, 0, channels, paFloat32, sr, blockSize, audioOutputCallback, param);
	param->stream = stream;
	param->channels = channels;
	//Make the flag:
	err = createAFlag(&param->running_flag);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	aFlagTestAndSet(param->running_flag);
	param->device = device;
	void* th;
	err = threadRun(audioOutputThread, param, &th);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	//let it go!
	*destination = param;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION void stopAudioOutputThread(void* thread) {
	aFlagClear(((ThreadParams*)thread)->running_flag);
}

void audioOutputThread(void* vparam) {
	ThreadParams *param = (ThreadParams*)vparam;
	LavDevice* device = param->device;
	LavCrossThreadRingBuffer *rb = param->ring_buffer;
	void* localMemoryManager = createMmanager();
	Pa_StartStream(param->stream);
	//This is simple.
	//Process one block from the device, write it to the ringbuffer, repeat.
	float* samples = mmanagerMalloc(localMemoryManager, param->block_size*sizeof(float)*param->channels);
	while(aFlagTestAndSet(param->running_flag)) {
		memset(samples, 0, param->block_size*sizeof(float)*param->channels);
		Lav_deviceGetBlock(device, samples);
		CTRBWriteItems(rb, param->block_size, samples);
		while(CTRBGetAvailableWrites(rb) <= param->block_size);
	}
	freeMmanager(localMemoryManager);
}

int audioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	//read length items into the output buffer.  That is it.
	CTRBGetItems(((ThreadParams*)userData)->ring_buffer, frameCount, output);
	return paNoError;
}

LavError portaudioDeviceConfigurer(LavDevice* device) {
	STANDARD_PREAMBLE;
	unsigned int blockSize, mixahead, sr, channels;
	blockSize = device->block_size;
	mixahead = device->mixahead;
	sr = device->sr;
	channels = device->channels;
	void* th; //the audio output thread.
	LavError err;
	err =  createAudioOutputThread(device, blockSize, channels, sr, mixahead, &th);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	device->device_specific_data = th;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}
