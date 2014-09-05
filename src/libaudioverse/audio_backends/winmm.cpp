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
	virtual void startup_hook();
	virtual void shutdown_hook();
	LavWinmmDevice(std::shared_ptr<LavSimulation> sim, unsigned int mixAhead, UINT_PTR which);
};

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
	return nullptr;
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
