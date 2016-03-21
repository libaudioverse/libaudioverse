/**This header is out-of-line because it is needed by the exactly two files in this directory.*/
#pragma once
//Removes windows min and max macros.'
#define NOMINMAX
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/com.hpp>
#include <mutex>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>


namespace audio_io {

class OutputDevice;

namespace implementation {

class LatencyPredictor;

class WasapiOutputDevice: public OutputDeviceImplementation {
	public:
	WasapiOutputDevice(std::function<void(float*, int)> callback, std::shared_ptr<IMMDevice> device, int inputFrames, int inputChannels, int inputSr, double minLatency, double startLatency, double maxLatency);
	~WasapiOutputDevice();
	void stop() override;
	private:
	void wasapiMixingThreadFunction();
	std::thread wasapi_mixing_thread;
	std::atomic_flag should_continue;
	SingleThreadedApartment sta; //Named so we can use APARTMENTCALL.
	std::shared_ptr<IAudioClient> client;
	std::shared_ptr<IMMDevice> device;
	//This can be Waveformatex, but we use the larger struct because sometimes it's not.
	WAVEFORMATEXTENSIBLE format;
	REFERENCE_TIME period = 0;
	double period_in_secs = 0.0;
	UINT32 wasapi_buffer_size;
	//We can't know ahead of time what the minimum latency we can deal with is, so we need to dynamically allocate the predictor.
	LatencyPredictor* latency_predictor = nullptr;
	//The rest of the variables live in the mixing thread.
};

class WasapiOutputDeviceFactory: public OutputDeviceFactoryImplementation {
	public:
	WasapiOutputDeviceFactory();
	~WasapiOutputDeviceFactory();
	std::vector<std::string> getOutputNames() override;
	std::vector<int> getOutputMaxChannels() override;
	std::unique_ptr<OutputDevice> createDevice(std::function<void(float*, int)> callback, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, float minLatency, float startLatency, float maxLatency) override;
	unsigned int getOutputCount() override;
	std::string getName();
	private:
	void rescan();
	std::vector<std::string> names;
	std::vector<int> max_channels;
	//Mmdevapi uses device identifier strings as opposed to integers, so we need to map.
	std::map<int, std::wstring> ids_to_id_strings;
	//The device enumerator, kept here so that we can avoid remaking it all the time.
	std::shared_ptr<IMMDeviceEnumerator> enumerator;
	SingleThreadedApartment sta;
};

//These are constants for the interfaces we care about.
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IMMDevice = __uuidof(IMMDevice);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioClock = __uuidof(IAudioClock);

}
}