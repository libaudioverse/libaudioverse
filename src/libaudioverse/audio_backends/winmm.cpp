/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/resampler.hpp>
#include <libaudioverse/private/errors.hpp>
#include <functional>
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
#include <windows.h>
#include <mmreg.h> //WAVEFORMATEXTENSIBLE

namespace libaudioverse_implementation {

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

class WinmmDevice: public  Device {
	public:
	//channels is what user requested, maxChannels is what the device can support at most.
	//maxChannels comes from the DeviceFactory subclass and is cached; thus the parameter here.
	WinmmDevice(std::function<void(float*, int)> getBuffer, unsigned int blockSize, unsigned int channels, unsigned int maxChannels, unsigned int mixAhead, UINT_PTR which, unsigned int sourceSr, unsigned int targetSr);
	virtual void startup_hook();
	virtual void shutdown_hook();
	void winmm_mixer();
	HWAVEOUT winmm_handle;
	HANDLE buffer_state_changed_event;
	std::thread winmm_mixing_thread;
	std::vector<WAVEHDR> winmm_headers;
	std::vector<short*> audio_data;
	std::atomic_flag winmm_mixing_flag;
};

WinmmDevice::WinmmDevice(std::function<void(float*, int)> getBuffer, unsigned int blockSize, unsigned int channels, unsigned int maxChannels, unsigned int mixAhead, UINT_PTR which, unsigned int sourceSr, unsigned int targetSr) {
	WAVEFORMATEXTENSIBLE format = {0};
	mixAhead += 1;
	winmm_headers.resize(mixAhead);
	audio_data.resize(mixAhead);
	buffer_state_changed_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(buffer_state_changed_event == NULL) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
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
		if(res != MMSYSERR_NOERROR) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
		outChannels = 2;
	}
	init(getBuffer, blockSize, sourceSr, outChannels, targetSr, mixAhead);
	for(unsigned int i = 0; i < audio_data.size(); i++) audio_data[i] = new short[output_buffer_size];
	//we can go ahead and set up the headers.
	for(unsigned int i = 0; i < winmm_headers.size(); i++) {
		winmm_headers[i].lpData = (LPSTR)audio_data[i];
		winmm_headers[i].dwBufferLength = sizeof(short)*blockSize*channels;
		winmm_headers[i].dwFlags = WHDR_DONE;
	}
	start();
}

void WinmmDevice::startup_hook() {
	winmm_mixing_flag.test_and_set();
	winmm_mixing_thread = std::thread([this]() {winmm_mixer();});
}

void WinmmDevice::shutdown_hook() {
	winmm_mixing_flag.clear();
	winmm_mixing_thread.join();
	if(winmm_handle)
	waveOutClose(winmm_handle);
}

void WinmmDevice::winmm_mixer() {
	float* workspace = new float[output_buffer_size];
	while(winmm_mixing_flag.test_and_set()) {
		while(1) {
			short* nextBuffer = nullptr;
			WAVEHDR* nextHeader = nullptr;
			for(unsigned int i = 0; i < winmm_headers.size(); i++) {
				if(winmm_headers[i].dwFlags & WHDR_DONE) {
					nextBuffer = audio_data[i];
					nextHeader = &winmm_headers[i];
					break;
				}
			}
			if(nextHeader == nullptr || nextBuffer == nullptr) break;
			zeroOrNextBuffer(workspace);
			waveOutUnprepareHeader(winmm_handle, nextHeader, sizeof(WAVEHDR));
			for(unsigned int i = 0; i < output_buffer_size; i++) nextBuffer[i] = (short)(workspace[i]*32767);
			nextHeader->dwFlags = 0;
			nextHeader->dwBufferLength = sizeof(short)*output_buffer_size;
			nextHeader->lpData = (LPSTR)nextBuffer;
			waveOutPrepareHeader(winmm_handle, nextHeader, sizeof(WAVEHDR));
			waveOutWrite(winmm_handle, nextHeader, sizeof(WAVEHDR));
		}
	WaitForSingleObject(buffer_state_changed_event, 5); //the timeout is to let us detect that we've been requested to die.
	}
}

class WinmmDeviceFactory: public DeviceFactory {
	public:
	WinmmDeviceFactory();
	virtual std::vector<std::string> getOutputNames();
	virtual std::vector<int> getOutputMaxChannels();
	virtual std::shared_ptr<Device> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead);
	virtual unsigned int getOutputCount();
	virtual bool scan();
	std::string getName();
	private:
	std::vector<std::string> names;
	std::vector<int> max_channels;
	std::vector<unsigned int> srs; //we need this, because these are not easy to query.
	unsigned int mapper_max_channels = 2, mapper_sr = 44100;
};

WinmmDeviceFactory::WinmmDeviceFactory() {
}

std::vector<std::string> WinmmDeviceFactory::getOutputNames() {
	return names;
}

std::vector<int> WinmmDeviceFactory::getOutputMaxChannels() {
	return max_channels;
}

std::shared_ptr<Device> WinmmDeviceFactory::createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) {
	//first, we need to do sanity checks.
	if(index < -1 || index > (int)names.size()) throw LavErrorException(Lav_ERROR_RANGE);
	std::shared_ptr<Device> device = std::make_shared<WinmmDevice>(getBuffer, blockSize, channels, index != -1 ? max_channels[index] : mapper_max_channels, mixAhead, index == -1 ? WAVE_MAPPER : index, sr, index == -1 ? mapper_sr : srs[index]);
	created_devices.push_back(device);
	return device;
}

unsigned int WinmmDeviceFactory::getOutputCount() {
	return names.size();
}

std::string WinmmDeviceFactory::getName() {
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
	retval.name = std::string(caps.szPname);
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

bool WinmmDeviceFactory::scan() {
	std::vector<std::string> newNames;
	std::vector<int> newMaxChannels;
	std::vector<unsigned int> newSrs; //we need this, because these are not easy to query.
	UINT devs = waveOutGetNumDevs();
	WinmmCapabilities caps;
	for(UINT i = 0; i < devs; i++) {
		caps = getWinmmCapabilities(i);
		//todo: unicode support
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

DeviceFactory* createWinmmDeviceFactory() {
	WinmmDeviceFactory* fact = new WinmmDeviceFactory();
	if(fact->scan() == false) {
		delete fact;
		return nullptr;
	}
	return fact;
}

}