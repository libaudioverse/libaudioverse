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
#include <windows.h>

class LavWinmmDevice: public  LavDevice {
	public:
	LavWinmmDevice(std::shared_ptr<LavSimulation> sim, unsigned int mixAhead, UINT_PTR which, WAVEFORMATEX format, unsigned int targetSr);
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

LavWinmmDevice::LavWinmmDevice(std::shared_ptr<LavSimulation> sim, unsigned int mixAhead, UINT_PTR which, WAVEFORMATEX format, unsigned int targetSr): LavDevice(sim, mixAhead) {
	mixAhead += 1;
	winmm_headers.resize(mixAhead);
	audio_data.resize(mixAhead);
	buffer_state_changed_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(buffer_state_changed_event == NULL) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	auto res = waveOutOpen(&winmm_handle, which, &format, (DWORD)buffer_state_changed_event, NULL, CALLBACK_EVENT);
	if(res != MMSYSERR_NOERROR) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	for(unsigned int i = 0; i < audio_data.size(); i++) audio_data[i] = new short[sim->getBlockSize()*sim->getChannels()]();
	//we can go ahead and set up the headers.
	for(unsigned int i = 0; i < winmm_headers.size(); i++) {
		winmm_headers[i].lpData = (LPSTR)audio_data[i];
		winmm_headers[i].dwBufferLength = sizeof(short)*sim->getBlockSize()*sim->getChannels();
		winmm_headers[i].dwFlags = WHDR_DONE;
	}
	init(targetSr);
	start();
}

void LavWinmmDevice::startup_hook() {
	winmm_mixing_flag.test_and_set();
	winmm_mixing_thread = std::thread([this]() {winmm_mixer();});
}

void LavWinmmDevice::shutdown_hook() {
	winmm_mixing_flag.clear();
	winmm_mixing_thread.join();
	waveOutClose(winmm_handle);
}

void LavWinmmDevice::winmm_mixer() {
	float* workspace = new float[simulation->getBlockSize()*simulation->getChannels()];
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
			for(unsigned int i = 0; i < simulation->getBlockSize()*simulation->getChannels(); i++) nextBuffer[i] = (short)(workspace[i]*32767);
			nextHeader->dwFlags = 0;
			nextHeader->dwBufferLength = sizeof(short)*simulation->getBlockSize()*simulation->getChannels();
			nextHeader->lpData = (LPSTR)nextBuffer;
			waveOutPrepareHeader(winmm_handle, nextHeader, sizeof(WAVEHDR));
			waveOutWrite(winmm_handle, nextHeader, sizeof(WAVEHDR));
		}
	WaitForSingleObject(buffer_state_changed_event, 5); //the timeout is to let us detect that we've been requested to die.
	}
}

class LavWinmmSimulationFactory: public LavSimulationFactory {
	public:
	LavWinmmSimulationFactory();
	virtual std::vector<std::string> getOutputNames();
	virtual std::vector<float> getOutputLatencies();
	virtual std::vector<int> getOutputMaxChannels();
	virtual std::shared_ptr<LavSimulation> createSimulation(int index, unsigned int sr, unsigned int blockSize, unsigned int mixAhead);
	virtual unsigned int getOutputCount();
	virtual bool scan();
	private:
	std::vector<float> latencies;
	std::vector<std::string> names;
	std::vector<int> max_channels;
	std::vector<unsigned int> srs; //we need this, because these are not easy to query.
};

unsigned int winmmExtractSr(WAVEOUTCAPS caps) {
	unsigned int sr = 0;
	if(caps.dwFormats && (WAVE_FORMAT_96S16 | WAVE_FORMAT_96M16)) {
		sr = 96000;
	} else if(caps.dwFormats & (WAVE_FORMAT_4S16 | WAVE_FORMAT_4M16)) {
		sr = 44100;
	} else if(caps.dwFormats & (WAVE_FORMAT_2S16 | WAVE_FORMAT_2M16)) {
		sr = 22050;
	} else if(caps.dwFormats  & (WAVE_FORMAT_1S16 | WAVE_FORMAT_1M16)) {
		sr = 11025;
	}
	return sr;
}

LavWinmmSimulationFactory::LavWinmmSimulationFactory() {
}

std::vector<std::string> LavWinmmSimulationFactory::getOutputNames() {
	return names;
}

std::vector<float> LavWinmmSimulationFactory::getOutputLatencies() {
	return latencies;
}

std::vector<int> LavWinmmSimulationFactory::getOutputMaxChannels() {
	return max_channels;
}

std::shared_ptr<LavSimulation> LavWinmmSimulationFactory::createSimulation(int index, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) {
	//first, we need to do sanity checks.
	if(index < -1 || index > (int)names.size()) throw LavErrorException(Lav_ERROR_RANGE);
	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = index != -1 ? max_channels[index] : 2;
	format.nSamplesPerSec = index != -1? srs[index] :44100;
	format.wBitsPerSample = 16;
	format.nAvgBytesPerSec = format.nSamplesPerSec*2;
	format.nBlockAlign = (format.nChannels*format.wBitsPerSample)/8;
	format.cbSize = 0;
	//create a simulation with the required parameters.
	std::shared_ptr<LavSimulation> retval = std::make_shared<LavSimulation>(sr, format.nChannels, blockSize, mixAhead);
	std::shared_ptr<LavWinmmDevice> device = std::make_shared<LavWinmmDevice>(retval, mixAhead, index == -1 ? WAVE_MAPPER : index, format, index == -1 ? 44100 : srs[index]);
	retval->associateDevice(device);
	return retval;
}

unsigned int LavWinmmSimulationFactory::getOutputCount() {
	return names.size();
}

bool LavWinmmSimulationFactory::scan() {
	std::vector<float> newLatencies;
	std::vector<std::string> newNames;
	std::vector<int> newMaxChannels;
	std::vector<unsigned int> newSrs; //we need this, because these are not easy to query.
	WAVEOUTCAPS caps;
	UINT devs = waveOutGetNumDevs();
	for(UINT i = 0; i < devs; i++) {
		waveOutGetDevCaps(i, &caps, sizeof(caps));
		//todo: unicode support
		std::string name(caps.szPname);
		//channels.
		int channels = caps.wChannels;
		unsigned int sr = 0;		
		sr = winmmExtractSr(caps);
		if(sr == 0) return false;
		newMaxChannels.push_back(channels);
		newNames.push_back(name);
		newSrs.push_back(sr);
		//we have no latency information.
		//This is too system specific. Consequently, we ahve to assume 0.
		newLatencies.push_back(0.0f);
	}
	this->max_channels = newMaxChannels;
	this->latencies = newLatencies;
	this->names = newNames;
	this->srs = newSrs;
	return true;
}

LavSimulationFactory* createWinmmSimulationFactory() {
	LavWinmmSimulationFactory* fact = new LavWinmmSimulationFactory();
	if(fact->scan() == false) {
		delete fact;
		return nullptr;
	}
	return fact;
}
