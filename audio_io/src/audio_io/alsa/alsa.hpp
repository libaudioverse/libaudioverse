#pragma once
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <alsa/asoundlib.h>

namespace audio_io {
namespace implementation {

class AlsaOutputDevice: public OutputDeviceImplementation {
	public:
	AlsaOutputDevice(std::function<void(float*, int)> callback, std::string name, int sr, int channels, int blockSize, float minLatency, float startLatency, float maxLatency);
	~AlsaOutputDevice();
	void stop();
	private:
	void workerThreadFunction();
	std::thread worker_thread;
	std::atomic_flag worker_running;
	std::string device_name;
	snd_pcm_t *device_handle = nullptr;
};

class AlsaOutputDeviceFactory: public OutputDeviceFactoryImplementation {
	public:
	AlsaOutputDeviceFactory();
	std::string getName() override;
	std::vector<std::string> getOutputNames() override;
	std::vector<int> getOutputMaxChannels() override;
	std::unique_ptr<OutputDevice> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, float minLatency, float startLatency, float maxLatency) override;
	private:
	void rescan();
	std::vector<std::string> device_names;
	std::vector<int> device_channels;
};

}
}