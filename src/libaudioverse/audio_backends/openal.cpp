/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/resampler.hpp>
#include <libaudioverse/private/errors.hpp>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <mutex>
#include <map>
#include <functional>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <al/al.h>
#include <al/alc.h>
#include <atomic>

namespace libaudioverse_implementation {

//Justification for ugliness and supurfluous comments: this is OpenAL.
//the mutex to make sure that nothing touches OpenAL while something else is.
std::mutex *openal_linearizer = nullptr;

class OpenALDevice: public  Device {
	public:
	virtual void startup_hook();
	virtual void shutdown_hook();
	OpenALDevice(std::function<void(float*, int)> getBuffer, unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixAhead, std::string which);
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

OpenALDevice::OpenALDevice(std::function<void(float*, int)> getBuffer, unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixAhead, std::string which) {
	auto lg = std::lock_guard<std::mutex>(*openal_linearizer);
	unsigned int outChannels = channels;
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
	if(outChannels == 8) { //can we get hold of 7.1?
		if(alIsExtensionPresent("AL_EXT_MCFORMATS") == AL_TRUE && (openAL71Format = alGetEnumValue("AL_FORMAT_71CHN16")) != 0) {
		//yes, this should be empty.
		}
		else outChannels = 6; //fall down to 5.1.
	}
	if(outChannels == 6) { //5.1
		if(alIsExtensionPresent("AL_EXT_MCFORMATS") == AL_TRUE && (openAL51Format = alGetEnumValue("AL_FORMAT_51CHN16")) != 0) {
		//yes, this should be empty.
		}
		else outChannels = 2; //fall down to stereo.
	}
	data_format = AL_FORMAT_MONO16;
	if(outChannels == 2) data_format = AL_FORMAT_STEREO16;
	else if(outChannels == 6) data_format = openAL51Format;
	else if(outChannels == 8) data_format = openAL71Format;
	samples_per_buffer = outChannels*blockSize;
	block = new float[samples_per_buffer];
	outgoing = new short[samples_per_buffer];
	sending_thread_sleep_time = (unsigned int)(((float)blockSize/sr)*1000);
	init(getBuffer, blockSize, sr, channels, sr, mixAhead);
	start();
}

void OpenALDevice::startup_hook() {
	sending_thread_continue.test_and_set();
	sending_thread = std::thread([this] () {sendingThreadFunction();});
}

void OpenALDevice::shutdown_hook() {
	sending_thread_continue.clear();
	sending_thread.join();
}

void OpenALDevice::sendingThreadFunction() {
	memset(block, 0, sizeof(float)*samples_per_buffer);
	//establish our mixahead.
	ALuint buff;
	for(auto i = buffers.begin(); i != buffers.end(); i++) {
		for(unsigned int j = 0; j < samples_per_buffer; j++) outgoing[j] = (short)(block[j]*32767);
		buff = *i;
		alBufferData(buff, data_format, outgoing, 2*samples_per_buffer, output_sr); //in this case, output_sr == input_sr always.
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
		alBufferData(buff, data_format, outgoing, 2*samples_per_buffer, output_sr); //in this case, target_sr == source_sr always.
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

class OpenALDeviceFactory: public DeviceFactory {
	public:
	OpenALDeviceFactory();
	virtual std::vector<std::string> getOutputNames();
	virtual std::vector<int> getOutputMaxChannels();
	virtual std::shared_ptr<Device> createDevice(std::function<void(float*, int)> getBlock, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead);
	std::string getName();
	private:
	std::vector<std::string> names;
	std::vector<int> max_channels;
	void scan();
};

OpenALDeviceFactory::OpenALDeviceFactory() {
	scan();
}

void OpenALDeviceFactory::scan() {
	std::vector<std::string> newNames;
	std::vector<int> newMaxChannels;
	const char* devices;
	ALCenum query = ALC_DEVICE_SPECIFIER;
	if(alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE) {
		query = alcGetEnumValue(nullptr, "ALC_ALL_DEVICES_SPECIFIER");
		if(query == AL_NONE) query = ALC_DEVICE_SPECIFIER;
	}
	devices = alcGetString(nullptr, query);
	unsigned int index = 0;
	while(devices[index]) {
		std::string name(&devices[index]);
		newNames.push_back(name);
		newMaxChannels.push_back(2);
		//move to the first character of the next device.
		index += name.size()+1;
	}
	names = newNames;
	max_channels = newMaxChannels;
	output_count = names.size();
}

std::string OpenALDeviceFactory::getName() {
	return "OpenAL";
}

std::vector<int>LavOpenALDeviceFactory::getOutputMaxChannels() {
	return max_channels;
}

std::vector<std::string> OpenALDeviceFactory::getOutputNames() {
	return names;
}

std::shared_ptr<Device> OpenALDeviceFactory::createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) {
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
	auto backend = std::make_shared<OpenALDevice>(getBuffer, sr, channels, blockSize, mixAhead, name);
	created_devices.push_back(backend);
	return backend;
}

DeviceFactory* createOpenALDeviceFactory() {
	openal_linearizer = new std::mutex();
	return new OpenALDeviceFactory();
}

}