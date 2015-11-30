//Windows.h defines min and max as macros, breaking std::min and std::max. This turns it off.
#define NOMINMAX
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/sample_format_converter.hpp>
#include <audio_io/private/latency_predictor.hpp>
#include <audio_io/private/logging.hpp>
#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <tuple>
#include <memory>
#include <utility>
#include <mutex>
#include <atomic>
#include <map>
#include <string.h>
#include <thread>
#include <chrono>
#include <windows.h>
#include <mmreg.h> //WAVEFORMATEXTENSIBLE

namespace audio_io {
namespace implementation {

/**This is the size of the winmm buffers in frames.*/
const int winmm_buffer_frames = 1024;
const double winmm_min_latency = 0.05, winmm_max_latency = 0.2;

WAVEFORMATEXTENSIBLE makeFormat(unsigned int channels, unsigned int sr, bool isExtended) {
	//lookup table so we can easily pull out masks.
	unsigned int chanmasks[] = {
		0,
		0,
		SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT,
		0,
		0,
		0,
		SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT,
		0,
		SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT,
	};
	WAVEFORMATEXTENSIBLE format;
	format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	format.Format.nSamplesPerSec = sr;
	format.Format.wBitsPerSample = 16;
	format.Format.cbSize = 22; //this comes directly from msdn, which gives no further explanation.
	format.Samples.wValidBitsPerSample = 16;
	format.Format.nAvgBytesPerSec = channels*2*sr;
	format.Format.nBlockAlign = channels*2;
	format.Format.nChannels = channels;
	format.dwChannelMask = chanmasks[channels];
	format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	if(isExtended == false) {
		format.Format.cbSize = 0;
		format.Format.wFormatTag = WAVE_FORMAT_PCM;
	}
	return format;
}

class WinmmOutputDevice: public  OutputDeviceImplementation {
	public:
	//channels is what user requested, maxChannels is what the device can support at most.
	//maxChannels comes from the DeviceFactory subclass and is cached; thus the parameter here.
	WinmmOutputDevice(std::function<void(float*, int)> getBuffer, unsigned int blockSize, unsigned int channels, unsigned int maxChannels, UINT_PTR which, unsigned int sourceSr, unsigned int targetSr, float minLatency, float startLatency, float maxLatency);
	~WinmmOutputDevice();
	void stop() override;
	void winmm_mixer();
	//Allocates or gets a recycled buffer from the unneeded queue.
	std::tuple<WAVEHDR*, short*> getWinmmBuffer();
	HWAVEOUT winmm_handle;
	HANDLE buffer_state_changed_event;
	std::thread winmm_mixing_thread;
	//Used to let us remember which buffer is last, etc.
	std::queue<std::tuple<WAVEHDR*, short*>> winmm_buffer_queue, winmm_unneeded_buffer_queue;
	std::atomic_flag winmm_mixing_flag;
	LatencyPredictor latency_predictor;
};

WinmmOutputDevice::WinmmOutputDevice(std::function<void(float*, int)> getBuffer, unsigned int blockSize, unsigned int channels, unsigned int maxChannels,  UINT_PTR which, unsigned int sourceSr, unsigned int targetSr, float minLatency, float startLatency, float maxLatency):
latency_predictor(30, std::max<double>(winmm_min_latency, minLatency), startLatency, std::min<double>(winmm_max_latency, maxLatency)) {
	WAVEFORMATEXTENSIBLE format = {0};
	buffer_state_changed_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(buffer_state_changed_event == NULL) {
		std::ostringstream format;
		format<<GetLastError();
		throw AudioIOError(std::string("Winmm: Could not create buffer_state_changed_event.  Windows error: ")+format.str());
	}
	//we try all the channels until we get a device, and then bale out if we've still failed.
	unsigned int chancounts[] = {8, 6, 2};
	MMRESULT res = 0;
	winmm_handle = nullptr;
	bool gotAudio = false;
	unsigned int neededChannels = channels < 2 ? 2 : channels;
	unsigned int inChannels = neededChannels, outChannels = 0;
	for(unsigned int i = 0; i < 3; i++) {
		if(chancounts[i] > neededChannels) continue;
		if(chancounts[i] > maxChannels) continue;
		format = makeFormat(chancounts[i], targetSr, true);
		res = waveOutOpen(&winmm_handle, which, (WAVEFORMATEX*)&format, (DWORD)buffer_state_changed_event, NULL, CALLBACK_EVENT);
		if(res == MMSYSERR_NOERROR) {
			gotAudio = true;
			outChannels = chancounts[i];
			break;
		}
	}
	if(gotAudio == false) { //we can still maybe get something by falling back to a last resort.
		//we make this back into a waveformatex and request stereo.
		format = makeFormat(2, targetSr, false);
		res = waveOutOpen(&winmm_handle, which, (WAVEFORMATEX*)&format, (DWORD)buffer_state_changed_event, NULL, CALLBACK_EVENT);
		if(res != MMSYSERR_NOERROR) throw AudioIOError("Could not open Winmm device with any attempted channel count.");
		outChannels = 2;
	}
	init(getBuffer, blockSize, channels, sourceSr, outChannels, targetSr);
	for(unsigned int i = 0; i < latency_predictor.predictLatencyInBlocks(winmm_buffer_frames, output_sr); i++) {
		winmm_buffer_queue.push(getWinmmBuffer());
	}
	winmm_mixing_flag.test_and_set();
	winmm_mixing_thread = std::thread([this]() {winmm_mixer();});
}

WinmmOutputDevice::~WinmmOutputDevice() {
	stop();
}

void WinmmOutputDevice::stop() {
	if(stopped) return;
	logInfo("Winmm device shutting down.");
	//Shut down the thread.
	if(winmm_mixing_thread.joinable()) {
		winmm_mixing_flag.clear();
		winmm_mixing_thread.join();
	}
	stopped = true;
}

std::tuple<WAVEHDR*, short*> WinmmOutputDevice::getWinmmBuffer() {
	short* buffer ;
	WAVEHDR *header;
	bool got = false; //Used so that we can fall through if the head of the queue is still being processed.
	if(winmm_unneeded_buffer_queue.size()) {
		std::tie(header, buffer) = winmm_unneeded_buffer_queue.front();
		if(header->dwFlags && WHDR_DONE) {
			got = true;
			winmm_unneeded_buffer_queue.pop();
		}
	}
	if(got == false) {
		header = new WAVEHDR();
		buffer = new short[winmm_buffer_frames*output_channels]();
	}
	header->lpData = (LPSTR)buffer;
	header->dwBufferLength = sizeof(short)*winmm_buffer_frames*output_channels;
	//Act as though we've already enqueued the buffer. This helps simplify the mixing thread.
	header->dwFlags = WHDR_DONE;
	return std::make_tuple(header, buffer);
}

void WinmmOutputDevice::winmm_mixer() {
	float* workspace = new float[winmm_buffer_frames*output_channels];
	logDebug("Winmm mixing thread started.");
	while(winmm_mixing_flag.test_and_set()) {
		int targetLatency = latency_predictor.predictLatencyInBlocks(winmm_buffer_frames, output_sr);
		short* nextBuffer = nullptr;
		WAVEHDR* nextHeader = nullptr;
		bool gotBuffer = false;
		if(targetLatency <= winmm_buffer_queue.size()) {
			//We might let one go shortly, but we have enough in circulation.
			std::tie(nextHeader, nextBuffer) = winmm_buffer_queue.front();
			//WHDR_DONE is set by the constructor for simplicity.
			if(nextHeader->dwFlags & WHDR_DONE) {
				winmm_buffer_queue.pop();
				gotBuffer = true;
			}
		}
		else {
			//We need to bring another buffer into circulation because our target latency is greater than the size of the queue.
			std::tie(nextHeader, nextBuffer) = getWinmmBuffer();
			gotBuffer = true;
		}
		if(gotBuffer) {
			latency_predictor.beginPass();
			sample_format_converter->write(winmm_buffer_frames, workspace);
			waveOutUnprepareHeader(winmm_handle, nextHeader, sizeof(WAVEHDR));
			for(unsigned int i = 0; i < winmm_buffer_frames*output_channels; i++) nextBuffer[i] = (short)(workspace[i]*32767);
			nextHeader->dwFlags = 0;
			nextHeader->dwBufferLength = sizeof(short)*winmm_buffer_frames*output_channels;
			nextHeader->lpData = (LPSTR)nextBuffer;
			waveOutPrepareHeader(winmm_handle, nextHeader, sizeof(WAVEHDR));
			waveOutWrite(winmm_handle, nextHeader, sizeof(WAVEHDR));
			latency_predictor.endPass();
			//Where the bufffer goes depends on the target latency: we might need to let it leave circulation.
			if(winmm_buffer_queue.size()+1 <= targetLatency) {
				winmm_buffer_queue.emplace(nextHeader, nextBuffer);
			} else {
				//We have too many in circulation, so we're going to let this one go.
				winmm_unneeded_buffer_queue.emplace(nextHeader, nextBuffer);
			}
		}
		WaitForSingleObject(buffer_state_changed_event, 5); //the timeout is to let us detect that we've been requested to die.
	}
	//we prepared these, we need to also kill them.  If we don't, very very bad things happen.
	//This call ends playback.
	waveOutReset(winmm_handle);
	decltype(winmm_buffer_queue) *queues[] = {&winmm_buffer_queue, &winmm_unneeded_buffer_queue};
	for(auto q: queues) {
		while(q->size()) {
			WAVEHDR *header;
			short *buffer;
			std::tie(header, buffer) = q->front();
			q->pop();
			//While the buffer is not done and it is enqueued or prepared.
			while((header->dwFlags & WHDR_DONE) == 0 && header->dwFlags & (WHDR_PREPARED||WHDR_INQUEUE)) {
				std::this_thread::yield();
			}
			waveOutUnprepareHeader(winmm_handle, header, sizeof(WAVEHDR));
			delete header;
			delete[] buffer;
		}
}
	waveOutClose(winmm_handle);
	winmm_handle=nullptr;
	logDebug("Winmm mixing thread exiting normally.");
}

class WinmmOutputDeviceFactory: public OutputDeviceFactoryImplementation {
	public:
	WinmmOutputDeviceFactory();
	virtual std::vector<std::string> getOutputNames() override;
	virtual std::vector<int> getOutputMaxChannels() override;
	virtual std::shared_ptr<OutputDevice> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, float minLatency, float startLatency, float maxLatency) override;
	virtual unsigned int getOutputCount() override;
	virtual bool scan();
	std::string getName() override;
	private:
	std::vector<std::string> names;
	std::vector<int> max_channels;
	std::vector<unsigned int> srs; //we need this, because these are not easy to query.
	unsigned int mapper_max_channels = 2, mapper_sr = 44100;
};

WinmmOutputDeviceFactory::WinmmOutputDeviceFactory() {
}

std::vector<std::string> WinmmOutputDeviceFactory::getOutputNames() {
	return names;
}

std::vector<int> WinmmOutputDeviceFactory::getOutputMaxChannels() {
	return max_channels;
}

std::shared_ptr<OutputDevice> WinmmOutputDeviceFactory::createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, float minLatency, float startLatency, float maxLatency) {
	std::shared_ptr<OutputDeviceImplementation> device = std::make_shared<WinmmOutputDevice>(getBuffer, blockSize, channels, index != -1 ? max_channels[index] : mapper_max_channels, index == -1 ? WAVE_MAPPER : index, sr, index == -1 ? mapper_sr : srs[index],
	minLatency, startLatency, maxLatency);
	created_devices.push_back(device);
	return device;
}

unsigned int WinmmOutputDeviceFactory::getOutputCount() {
	return names.size();
}

std::string WinmmOutputDeviceFactory::getName() {
	return "Winmm";
}

struct WinmmCapabilities {
	unsigned int sr;
	std::string name;
	unsigned int channels;
};

WinmmCapabilities getWinmmCapabilities(UINT index) {
	WAVEFORMATEXTENSIBLE format;
	WAVEOUTCAPS caps;
	waveOutGetDevCaps(index, &caps, sizeof(caps));
	WinmmCapabilities retval;
	retval.sr = 44100;
	retval.channels = 2;
	//Winmm is old enough that it uses TChar, which changes depending on this define.
	#ifdef UNICODE
	auto name = std::wstring(caps.szPname);
	//We use this function twice. First time to get the size.
	int length = WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, NULL, 0, NULL, NULL);
	//It's unclear if we're responsible for reserving the null character or not, so we do it anyway for safety.
	char* buffer = new char[length+1]();
	WideCharToMultiByte(CP_UTF8, 0, name.c_str(), -1, buffer, length, NULL, NULL);
	buffer[length] = '\0';
	retval.name = std::string(buffer);
	delete[] buffer;
	#else
	retval.name = std::string(caps.szPname);
	#endif
	unsigned int srs[] = {48000, 44100, 22050};
	unsigned int srsCount = 3;
	unsigned int channels[] = {8, 6, 2};
	unsigned int channelsCount = 3;
	for(unsigned int i = 0; i < channelsCount; i++) {
		for(unsigned int j = 0; j < srsCount; j++) {
			format = makeFormat(channels[i], srs[j], true);
			auto res = waveOutOpen(NULL, index, (WAVEFORMATEX*)&format, NULL, NULL, WAVE_FORMAT_QUERY);
			if(res == MMSYSERR_NOERROR) {
				retval.sr = srs[j];
				retval.channels = channels[i];
				goto done;
			}
		}
	}
	done:
	return retval;
}

bool WinmmOutputDeviceFactory::scan() {
	std::vector<std::string> newNames;
	std::vector<int> newMaxChannels;
	std::vector<unsigned int> newSrs; //we need this, because these are not easy to query.
	UINT devs = waveOutGetNumDevs();
	WinmmCapabilities caps;
	for(UINT i = 0; i < devs; i++) {
		caps = getWinmmCapabilities(i);
		std::string name(caps.name);
		//channels.
		unsigned int channels = caps.channels;
		unsigned int sr = caps.sr;
		newMaxChannels.push_back(channels);
		newNames.push_back(name);
		newSrs.push_back(sr);
	}
	this->max_channels = newMaxChannels;
	this->names = newNames;
	this->srs = newSrs;
	caps = getWinmmCapabilities(WAVE_MAPPER);
	mapper_max_channels = caps.channels;
	mapper_sr = caps.sr;
	return true;
}

OutputDeviceFactory* createWinmmOutputDeviceFactory() {
	WinmmOutputDeviceFactory* fact = new WinmmOutputDeviceFactory();
	if(fact->scan() == false) {
		delete fact;
		return nullptr;
	}
	return fact;
}

} //end namespace implementation
} //end namespace audio_io