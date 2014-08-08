/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_physical_outputs.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_resampler.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_errors.hpp>
#include <string>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <mutex>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>

/**Public facing code.  This includes the rest of the library itself and the public API.*/

//list from greatest to least priority.
LavPhysicalOutputFactoryCreationFunction possible_backends[] = {
	createPortaudioPhysicalOutputFactory,
};
LavPhysicalOutputFactory* chosen_factory = nullptr;


void initializePhysicalOutputFactory() {
	for(unsigned int i = 0; i < sizeof(possible_backends)/sizeof(LavPhysicalOutputFactoryCreationFunction); i++) {
		LavPhysicalOutputFactory* possible = possible_backends[i]();
		if(possible != nullptr) {
			chosen_factory = possible;
			return;
		}
	}
	//none of them returned a factory, so we need to crash and burn.
	throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_getPhysicalOutputCount(unsigned int* destination) {
	PUB_BEGIN
	*destination = chosen_factory->getOutputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_getPhysicalOutputLatency(unsigned int index, float* destination) {
	PUB_BEGIN
	auto l = chosen_factory->getOutputLatencies();
	if(index >= l.size()) throw LavErrorException(Lav_ERROR_RANGE);
	*destination = l[index];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_getPhysicalOutputName(unsigned int index, char** destination) {
	PUB_BEGIN
	auto n = chosen_factory->getOutputNames();
	if(index >= n.size()) throw LavErrorException(Lav_ERROR_RANGE);
	auto s = n[index];
	char* outgoingStr = new char[s.size()];
	std::copy(s.c_str(), s.c_str()+s.size(), outgoingStr);
	std::shared_ptr<char> outgoing(outgoingStr, [](char* what){delete[] what;});
	*destination = outgoingPointer<char>(outgoing);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_getPhysicalOutputChannels(unsigned int index, unsigned int* destination) {
	PUB_BEGIN
	auto c = chosen_factory->getOutputMaxChannels();
	if(index >= c.size()) throw LavErrorException(Lav_ERROR_RANGE);
	*destination = c[index];
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError lav_createDeviceForOutput(int index, unsigned int sr, unsigned int blockSize, unsigned int mixAhead, LavDevice** destination) {
	PUB_BEGIN
	auto dev = chosen_factory->createDevice(index, sr, blockSize, mixAhead);
	*destination = outgoingPointer<LavDevice>(dev);
	PUB_END
}
