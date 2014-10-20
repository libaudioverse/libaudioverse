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
#include <memory>
#include <utility>
#include <mutex>
#include <map>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <al/al.h>
#include <al/alc.h>
#include <atomic>

//Justification for ugliness and supurfluous comments: this is OpenAL.
//the mutex to make sure that nothing touches OpenAL while something else is.
std::mutex *openal_linearizer = nullptr;

class LavOpenALDevice: public  LavDevice {
	public:
	virtual void startup_hook();
	virtual void shutdown_hook();
	LavOpenALDevice(std::shared_ptr<LavSimulation> sim, unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixAhead, std::string which);
	void sendingThreadFunction();
	ALCdevice* device = nullptr;
	ALCcontext *context = nullptr;
	ALuint source;
	std::vector<ALuint> buffers;
	float* block = nullptr;
	short* outgoing = nullptr;
	unsigned int samples_per_buffer = 0;
	ALenum data_format = 0;
	std::atomic_flag sending_thread_continue;
	std::thread sending_thread;
	unsigned int sending_thread_sleep_time = 0;
};

LavOpenALDevice::LavOpenALDevice(std::shared_ptr<LavSimulation> sim, unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixAhead, std::string which): LavDevice(sim, mixAhead) {
	auto lg = std::lock_guard<std::mutex>(*openal_linearizer);
	device = alcOpenDevice(which.c_str());
	if(device == nullptr)  throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	context = alcCreateContext(device, nullptr);
	if(context == nullptr) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	ALenum err;
	if(alcMakeContextCurrent(context) == ALC_FALSE) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	alGenSources(1, &source);
	err = alGetError();
	if(err != AL_NONE) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	for(unsigned int i = 0; i < mixAhead; i++) {
		ALuint buff;
		alGenBuffers(1, &buff);
		err = alGetError();
		if(err != AL_NONE) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
		buffers.push_back(buff);
	}
	ALuint openAL51Format, openAL71Format;
	if(channels == 8) { //can we get hold of 7.1?
		if(alIsExtensionPresent("AL_EXT_MCFORMATS") == AL_TRUE && (openAL71Format = alGetEnumValue("AL_FORMAT_71CHN16")) != 0) {
		//yes, this should be empty.
		}
		else channels = 6; //fall down to 5.1.
	}
	if(channels == 6) { //5.1
		if(alIsExtensionPresent("AL_EXT_MCFORMATS") == AL_TRUE && (openAL51Format = alGetEnumValue("AL_FORMAT_51CHN16")) != 0) {
		//yes, this should be empty.
		}
		else channels = 2; //fall down to stereo.
	}
	data_format = AL_FORMAT_MONO16;
	if(channels == 2) data_format = AL_FORMAT_STEREO16;
	else if(channels == 6) data_format = openAL51Format;
	else if(channels == 8) data_format = openAL71Format;
	samples_per_buffer = channels*blockSize;
	block = new float[samples_per_buffer];
	outgoing = new short[samples_per_buffer];
	sending_thread_sleep_time = (unsigned int)(((float)blockSize/sr)*1000);
	init(sr, channels);
	start();
}

void LavOpenALDevice::startup_hook() {
	sending_thread_continue.test_and_set();
	sending_thread = std::thread([this] () {sendingThreadFunction();});
}

void LavOpenALDevice::shutdown_hook() {
	sending_thread_continue.clear();
	sending_thread.join();
}

void LavOpenALDevice::sendingThreadFunction() {
	memset(block, 0, sizeof(float)*samples_per_buffer);
	//establish our mixahead.
	ALuint buff;
	for(auto i = buffers.begin(); i != buffers.end(); i++) {
		for(unsigned int j = 0; j < samples_per_buffer; j++) outgoing[j] = (short)(block[j]*32767);
		buff = *i;
		alBufferData(buff, data_format, outgoing, 2*samples_per_buffer, target_sr); //in this case, target_sr == source_sr always.
	}
	//enqueue everything in the buffers vector.
	openal_linearizer->lock();
	alSourceQueueBuffers(source, buffers.size(), &buffers[0]);
	alSourcePlay(source);
	alGetError();
	openal_linearizer->unlock();
	bool hasBlock = false;
	while(sending_thread_continue.test_and_set()) {
		if(hasBlock == false) {
			zeroOrNextBuffer(block);
			for(unsigned int i = 0; i < samples_per_buffer; i++) outgoing[i] = (short)(block[i]*32767);
			hasBlock = true;
		}
		openal_linearizer->lock();
		alGetError(); //make sure to clear it.
		if(alcMakeContextCurrent(context) == AL_FALSE) {
			openal_linearizer->unlock();
			continue;
		}
		alSourceUnqueueBuffers(source, 1, &buff);
		if(alGetError() == AL_INVALID_VALUE) {
			openal_linearizer->unlock();
			continue;
		}
		alBufferData(buff, data_format, outgoing, 2*samples_per_buffer, target_sr); //in this case, target_sr == source_sr always.
		if(alGetError() != AL_NONE) {
			openal_linearizer->unlock();
			continue;
		}
		alSourceQueueBuffers(source, 1, &buff);
		if(alGetError() != AL_NONE) {
			openal_linearizer->unlock();
			continue;
		}
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if(state != AL_PLAYING) alSourcePlay(source);
		hasBlock = false;
		alGetError();
		openal_linearizer->unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(sending_thread_sleep_time));
	}
}

class LavOpenALSimulationFactory: public LavSimulationFactory {
	public:
	LavOpenALSimulationFactory();
	virtual std::vector<std::string> getOutputNames();
	virtual std::vector<float> getOutputLatencies();
	virtual std::vector<int> getOutputMaxChannels();
	virtual std::shared_ptr<LavSimulation> createSimulation(int index, bool useDefaults, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead);
	std::string getName();
	private:
	std::vector<std::string> names;
	std::vector<float> latencies;
	std::vector<int> max_channels;
	void scan();
};

LavOpenALSimulationFactory::LavOpenALSimulationFactory() {
	scan();
}

void LavOpenALSimulationFactory::scan() {
	std::vector<std::string> newNames;
	std::vector<float> newLatencies;
	std::vector<int> newMaxChannels;
	const char* devices;
	devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
	unsigned int index = 0;
	while(devices[index]) {
		std::string name(&devices[index]);
		newNames.push_back(name);
		newLatencies.push_back(-1.0f);
		newMaxChannels.push_back(2);
		//move to the first character of the next device.
		index += name.size()+1;
	}
	names = newNames;
	max_channels = newMaxChannels;
	latencies = newLatencies;
}

std::string LavOpenALSimulationFactory::getName() {
	return "OpenAL";
}

std::vector<float> LavOpenALSimulationFactory::getOutputLatencies() {
	return latencies;
}

std::vector<int>LavOpenALSimulationFactory::getOutputMaxChannels() {
	return max_channels;
}

std::vector<std::string> LavOpenALSimulationFactory::getOutputNames() {
	return names;
}

std::shared_ptr<LavSimulation> LavOpenALSimulationFactory::createSimulation(int index, bool useDefaults, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) {
	if(useDefaults) {
		channels = 2;
		sr = 44100;
		mixAhead = 6;
		blockSize = 512;
	}
	std::string name;
	if(index < -1 || index >= (int)names.size()) throw LavErrorException(Lav_ERROR_RANGE);
	if(index == -1) {
		name = std::string(alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER));
	}
	else {
		name = names[index];
	}
	if(((channels == 1 || channels == 2
|| channels == 6 ||channels == 8) && sr != 0 && blockSize != 0) == false)throw LavErrorException(Lav_ERROR_RANGE);
	auto sim = std::make_shared<LavSimulation>(sr, blockSize, mixAhead);
	auto backend = std::make_shared<LavOpenALDevice>(sim, sr, channels, blockSize, mixAhead, name);
	sim->associateDevice(backend);
	return sim;
}

LavSimulationFactory* createOpenALSimulationFactory() {
	openal_linearizer = new std::mutex();
	return new LavOpenALSimulationFactory();
}
