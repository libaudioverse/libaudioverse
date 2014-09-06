/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_audio_devices.hpp>
#include <libaudioverse/private_simulation.hpp>
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

int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

class LavPortaudioDevice: public  LavDevice {
	public:
	virtual void startup_hook();
	virtual void shutdown_hook();
	LavPortaudioDevice(std::shared_ptr<LavSimulation> sim, unsigned int mixAhead, PaDeviceIndex which);
	PaStream* stream = nullptr;
	friend int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);
};

class LavPortaudioSimulationFactory: public LavSimulationFactory {
	public:
	LavPortaudioSimulationFactory();
	virtual std::vector<std::string> getOutputNames();
	virtual std::vector<float> getOutputLatencies();
	virtual std::vector<int> getOutputMaxChannels();
	virtual std::shared_ptr<LavSimulation> createSimulation(int index, unsigned int sr, unsigned int blockSize, unsigned int mixAhead);
	private:
	std::map<unsigned int, PaDeviceIndex> output_indices_map;
	std::vector<float> latencies;
	std::vector<std::string> names;
	std::vector<int> max_channels;
};

LavPortaudioDevice::LavPortaudioDevice(std::shared_ptr<LavSimulation> sim, unsigned int mixAhead, PaDeviceIndex which): LavDevice(sim, mixAhead) {
	const PaDeviceInfo* devinfo = Pa_GetDeviceInfo(which);
	PaStreamParameters params;
	params.channelCount = sim->getChannels();
	params.device = which;
	params.hostApiSpecificStreamInfo = nullptr;
	params.sampleFormat = paFloat32;
	params.suggestedLatency = devinfo->defaultLowOutputLatency;
	double sr = devinfo->defaultSampleRate;
	init((unsigned int)sr);
	PaError err = Pa_OpenStream(&stream, nullptr, &params, sr, output_buffer_size/simulation->getChannels(), 0, portaudioOutputCallback, this);
	if(err != paNoError) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	start();
}

void LavPortaudioDevice::startup_hook() {
	Pa_StartStream(stream);
}

void LavPortaudioDevice::shutdown_hook() {
	Pa_StopStream(stream);
}

int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	LavPortaudioDevice * const out = (LavPortaudioDevice*)userData;
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

LavPortaudioSimulationFactory::LavPortaudioSimulationFactory() {
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

std::vector<std::string> LavPortaudioSimulationFactory::getOutputNames() {
	return names;
}

std::vector<float> LavPortaudioSimulationFactory::getOutputLatencies() {
	return latencies;
}

std::vector<int> LavPortaudioSimulationFactory::getOutputMaxChannels() {
	return max_channels;
}

std::shared_ptr<LavSimulation> LavPortaudioSimulationFactory::createSimulation(int index, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) {
	if(index != -1 || index >= output_count) throw LavErrorException(Lav_ERROR_RANGE);
	//if it's not -1, then we can cast it to a PaDeviceIndex.  Otherwise, we use Pa_GetDefaultOutputDevice();
	PaDeviceIndex needed;
	if(index == -1) needed = Pa_GetDefaultOutputDevice();
	else needed = output_indices_map[index];
	//we create a device, first.
	std::shared_ptr<LavSimulation> retval = std::make_shared<LavSimulation>(sr, index != -1 ? max_channels[index] : 2, blockSize, mixAhead);
	//create the output.
	std::shared_ptr<LavPortaudioDevice> output = std::make_shared<LavPortaudioDevice>(retval, mixAhead, needed);
	retval->associateDevice(output);
	return retval;
}

LavSimulationFactory* createPortaudioSimulationFactory() {
	Pa_Initialize();
	return new LavPortaudioSimulationFactory();
}
