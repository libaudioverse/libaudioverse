#include "alsa.hpp"
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <string>
#include <vector>
#include <memory>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

namespace audio_io {
namespace implementation {

AlsaOutputDeviceFactory::AlsaOutputDeviceFactory() {
	rescan();
}

std::string AlsaOutputDeviceFactory::getName() {
	return "alsa";
}

std::vector<std::string> AlsaOutputDeviceFactory::getOutputNames() {
	return device_names;
}

std::vector<int> AlsaOutputDeviceFactory::getOutputMaxChannels() {
	return device_channels;
}

std::unique_ptr<OutputDevice> AlsaOutputDeviceFactory::createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, int mixahead) {
	std::string n;
	if(index == -1) n = "default";
	else n = device_names[index];
	return std::unique_ptr<OutputDevice>(new AlsaOutputDevice(getBuffer, n, sr, channels, blockSize, mixahead));
}

void AlsaOutputDeviceFactory::rescan() {
	device_names.clear();	
	device_channels.clear();
	//In the following, we always assume that the device supports 8 channels.
	//Todo: can we get more info out of Alsa somehow?
	void** hints = nullptr;
	auto hint_success = snd_device_name_hint(-1, "pcm", &hints);
	if(hint_success != 0 || hints == nullptr) {
		output_count = 0;
		return; //Advertise no devices because scanning failed.
	}
	for(int i = 0; hints[i] != nullptr; i++) {
		auto hint = hints[i];
		auto name = snd_device_name_get_hint(hint, "NAME");
		if(name == nullptr) continue; 
		auto desc = snd_device_name_get_hint(hint, "DESC");
		auto ioid = snd_device_name_get_hint(hint, "IOID");
		if(ioid != nullptr && strcmp(ioid, "Input") != 0) goto freeing;
		device_names.push_back(std::string(name));
		device_channels.push_back(8); //Can we do better?
		freeing:
		if(name) free(name);
		if(desc) free(desc);
		if(ioid) free(ioid);
	}
	snd_device_name_free_hint(hints);
	output_count = device_names.size();
}

OutputDeviceFactory* createAlsaOutputDeviceFactory() {
	return new AlsaOutputDeviceFactory();
}

}
}