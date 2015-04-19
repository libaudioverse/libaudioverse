/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/resampler.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/errors.hpp>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <mutex>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <libaudioverse/private/logging.hpp>

/**Public facing code.  This includes the rest of the library itself and the public API.*/

//list from greatest to least priority.
LavDeviceFactoryCreationFunction possible_backends[] = {
	createWinmmDeviceFactory,
//	createOpenALDeviceFactory,
};
LavDeviceFactory* chosen_factory = nullptr;

void initializeDeviceFactory() {
	log(Lav_LOG_LEVEL_INFO, "Initializing audio backend.");
	for(unsigned int i = 0; i < sizeof(possible_backends)/sizeof(LavDeviceFactoryCreationFunction); i++) {
		LavDeviceFactory* possible = possible_backends[i]();
		if(possible != nullptr) {
			chosen_factory = possible;
			log(Lav_LOG_LEVEL_INFO, "Chosen backend is %s", chosen_factory->getName().c_str());
			return;
		}
	}
	//none of them returned a factory, so we need to crash and burn.
	throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
}

void shutdownDeviceFactory() {
	if(chosen_factory) delete chosen_factory;
}


//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetCount(unsigned int* destination) {
	PUB_BEGIN
	*destination = chosen_factory->getOutputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetLatency(unsigned int index, float* destination) {
	PUB_BEGIN
	auto l = chosen_factory->getOutputLatencies();
	if(index >= l.size()) throw LavErrorException(Lav_ERROR_RANGE);
	*destination = l[index];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetName(unsigned int index, char** destination) {
	PUB_BEGIN
	auto n = chosen_factory->getOutputNames();
	if(index >= n.size()) throw LavErrorException(Lav_ERROR_RANGE);
	auto s = n[index];
	char* outgoingStr = new char[s.size()+1];
	std::copy(s.c_str(), s.c_str()+s.size(), outgoingStr);
	outgoingStr[s.size()] = '\0';
	std::shared_ptr<char> outgoing(outgoingStr, [](char* what){delete[] what;});
	*destination = outgoingPointer<char>(outgoing);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetChannels(unsigned int index, unsigned int* destination) {
	PUB_BEGIN
	auto c = chosen_factory->getOutputMaxChannels();
	if(index >= c.size()) throw LavErrorException(Lav_ERROR_RANGE);
	*destination = c[index];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_createSimulationForDevice(int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead, LavHandle* destination) {
	PUB_BEGIN
	//don't use defaults, use user options.
	auto sim = std::make_shared<LavSimulation>(sr, blockSize, mixAhead);
	sim->completeInitialization();
	//this can in theory invalidate while the callback is still needed:
	auto weak_sim = std::weak_ptr<LavSimulation>(sim);
	auto audioFunction = [=](float* out, int channels) {
		auto strong_sim = weak_sim.lock();
		if(strong_sim == nullptr) {
			memset(out, 0, sizeof(float)*channels*blockSize);
			return;
		}
		strong_sim->lock();
		strong_sim->getBlock(out, channels, false);
		strong_sim->unlock();
	};
	auto dev = chosen_factory->createDevice(audioFunction, index, channels, sr, blockSize, mixAhead);
	sim->associateDevice(dev);
	*destination = outgoingObject<LavSimulation>(sim);
	PUB_END
}

//the special case of a device without an output.
Lav_PUBLIC_FUNCTION LavError Lav_createReadSimulation(unsigned int sr, unsigned int blockSize, LavHandle* destination) {
	PUB_BEGIN
	auto shared = std::make_shared<LavSimulation>(sr, blockSize, 0);
	shared->completeInitialization();
	*destination = outgoingObject(shared);
	PUB_END
}
