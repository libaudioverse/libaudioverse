/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_devices.hpp>
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <atomic>

int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

class LavPortaudioDevice: LavDevice {
	std::thread audioOutputThread;
	std::atomic_flag runningFlag; //when this clears, the audio thread self-terminates.
	PaStream *stream; //the portaudio stream we work with.  Started and stopped by the background thread for us.
	//this is used to hand buffers to the audio callback.
	//the audio callback simply sets it to NULL at the end.  We can use CAS on it to pass buffers in as needed.
	std::atomic<float*> outgoing_buffer;
	friend int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);
};

LavError initializeAudioBackend() {
	PaError err = Pa_Initialize();
	if(err < 0) {
		return Lav_ERROR_CANNOT_INIT_AUDIO;
	}
	return Lav_ERROR_NONE;
}


int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	LavPortaudioDevice * const dev = (LavPortaudioDevice*)userData;
	//we need to see if a buffer is ready for us.
	const float* buff = dev->outgoing_buffer.load();
	if(buff) {
		memcpy(output, buff, sizeof(float)*frameCount);
	}
	else {
		memset(output, 0, frameCount*sizeof(float));
		}
	//clear the flag, signalling that we should get the next one ASAP.
	dev->outgoing_buffer.store(nullptr);
	return paContinue;
}
