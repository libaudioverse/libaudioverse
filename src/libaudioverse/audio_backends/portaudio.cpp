/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_physical_outputs.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_resampler.hpp>
#include <libaudioverse/private_errors.hpp>
#include <string>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <mutex>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <portaudio.h>

int portaudioOutputCallbackB(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

class LavPortaudioPhysicalOutput: public  LavPhysicalOutput {
	public:
	virtual void startup_hook();
	virtual void shutdown_hook();
	LavPortaudioPhysicalOutput(std::shared_ptr<LavDevice> dev, unsigned int mixAhead, PaDeviceIndex which);
	PaStream* stream = nullptr;
	friend int portaudioOutputCallbackB(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);
};

LavPortaudioPhysicalOutput::LavPortaudioPhysicalOutput(std::shared_ptr<LavDevice> dev, unsigned int mixAhead, PaDeviceIndex which):  LavPhysicalOutput(dev, mixAhead) {
	const PaDeviceInfo* devinfo = Pa_GetDeviceInfo(which);
	PaStreamParameters params;
	params.channelCount = dev->getChannels();
	params.device = which;
	params.hostApiSpecificStreamInfo = nullptr;
	params.sampleFormat = paFloat32;
	params.suggestedLatency = devinfo->defaultLowOutputLatency;
	double sr = devinfo->defaultSampleRate;
	PaError err = Pa_OpenStream(&stream, nullptr, &params, sr, dev->getBlockSize(), 0, portaudioOutputCallbackB, this);
	if(err != paNoError) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	init((unsigned int)sr);
}

void LavPortaudioPhysicalOutput::startup_hook() {
	Pa_StartStream(stream);
}

void LavPortaudioPhysicalOutput::shutdown_hook() {
	Pa_StopStream(stream);
}

int portaudioOutputCallbackB(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	LavPortaudioPhysicalOutput * const out = (LavPortaudioPhysicalOutput*)userData;
	const int haveBuffer = out->buffer_statuses[out->callback_buffer_index].load();
	if(haveBuffer) {
		memcpy(output, out->buffers[out->callback_buffer_index], sizeof(float)*frameCount*out->channels);
		out->buffer_statuses[out->callback_buffer_index].store(0);
		out->callback_buffer_index++;
		out->callback_buffer_index %= out->mix_ahead+1;
	}
	else {
		memset(output, 0, out->channels*frameCount*sizeof(float));
	}
	return paContinue;
}
