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
#include <map>
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

class LavPortaudioPhysicalOutputFactory: public LavPhysicalOutputFactory {
	public:
	LavPortaudioPhysicalOutputFactory();
	virtual std::vector<std::string> getOutputNames();
	virtual std::vector<float> getOutputLatencies();
	virtual std::vector<int> getOutputMaxChannels();
	virtual std::shared_ptr<LavDevice> createDevice(int index, unsigned int sr, unsigned int blockSize, unsigned int mixAhead);
	private:
	std::map<unsigned int, PaDeviceIndex> output_indices_map;
	std::vector<float> latencies;
	std::vector<std::string> names;
	std::vector<int> max_channels;
};

LavPortaudioPhysicalOutput::LavPortaudioPhysicalOutput(std::shared_ptr<LavDevice> dev, unsigned int mixAhead, PaDeviceIndex which): LavPhysicalOutput(dev, mixAhead) {
	const PaDeviceInfo* devinfo = Pa_GetDeviceInfo(which);
	PaStreamParameters params;
	params.channelCount = dev->getChannels();
	params.device = which;
	params.hostApiSpecificStreamInfo = nullptr;
	params.sampleFormat = paFloat32;
	params.suggestedLatency = devinfo->defaultLowOutputLatency;
	double sr = devinfo->defaultSampleRate;
	init((unsigned int)sr);
	PaError err = Pa_OpenStream(&stream, nullptr, &params, sr, output_buffer_size, 0, portaudioOutputCallbackB, this);
	if(err != paNoError) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	start();
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

LavPortaudioPhysicalOutputFactory::LavPortaudioPhysicalOutputFactory() {
	//recall that portaudio doesn't rescan, so we do all our scanning here.
	unsigned int index = 0;
	for(PaDeviceIndex i = 0; i < Pa_GetDeviceCount()-1; i++) {
		const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
		if(info->maxOutputChannels == 0) continue;
		names.emplace_back(info->name);
		latencies.emplace_back((float)info->defaultLowOutputLatency);
		max_channels.emplace_back(info->maxOutputChannels);
		output_indices_map[index] = i;
		index++;
	}
	output_count = names.size();
}

std::vector<std::string> LavPortaudioPhysicalOutputFactory::getOutputNames() {
	return names;
}

std::vector<float> LavPortaudioPhysicalOutputFactory::getOutputLatencies() {
	return latencies;
}

std::vector<int> LavPortaudioPhysicalOutputFactory::getOutputMaxChannels() {
	return max_channels;
}

std::shared_ptr<LavDevice> LavPortaudioPhysicalOutputFactory::createDevice(int index, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) {
	if(index != -1 || index >= output_count) throw LavErrorException(Lav_ERROR_RANGE);
	//if it's not -1, then we can cast it to a PaDeviceIndex.  Otherwise, we use Pa_GetDefaultOutputDevice();
	PaDeviceIndex needed;
	if(index == -1) needed = Pa_GetDefaultOutputDevice();
	else needed = output_indices_map[index];
	//we create a device, first.
	std::shared_ptr<LavDevice> retval = std::make_shared<LavDevice>(sr, index != -1 ? max_channels[index] : 2, blockSize, mixAhead);
	//create the output.
	std::shared_ptr<LavPortaudioPhysicalOutput> output = std::make_shared<LavPortaudioPhysicalOutput>(retval, mixAhead, needed);
	retval->associateOutput(output);
	return retval;
}

LavPhysicalOutputFactory* createPortaudioPhysicalOutputFactory() {
	Pa_Initialize();
	return new LavPortaudioPhysicalOutputFactory();
}