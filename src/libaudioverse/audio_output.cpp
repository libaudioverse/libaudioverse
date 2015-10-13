/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/error.hpp>
#include <libaudioverse/private/logging.hpp>
#include <audio_io/audio_io.hpp>
#include <logger_singleton/logger_singleton.hpp>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <algorithm>
#include <thread>


namespace libaudioverse_implementation {

std::shared_ptr<audio_io::OutputDeviceFactory> *audio_output_factory;

void initializeDeviceFactory() {
	try {
		logInfo("Initializing audio backend.");
		//Hook audio_io's logger to our logger.
		audio_io::getLogger()->setAsForwarder(getLogger());
		audio_io::initialize();
		audio_output_factory = new std::shared_ptr<audio_io::OutputDeviceFactory>();
		auto possible=audio_io::getOutputDeviceFactory();
		if(possible != nullptr) {
			*audio_output_factory = possible;
			logInfo("Chosen backend is %s", (*audio_output_factory)->getName().c_str());
			return;
		}
		else {
			ERROR(Lav_ERROR_CANNOT_INIT_AUDIO, "Not able to initialize an audio backend.");
		}
	}
	catch(std::exception &e) {
		//We should catch specific audio_io exceptions here, but they all inherrit from std::exception and audio_io does not differentiate as of yet.
		ERROR(Lav_ERROR_CANNOT_INIT_AUDIO, e.what());
	}
}

void shutdownDeviceFactory() {
	if(audio_output_factory) delete audio_output_factory;
	audio_io::shutdown();
}

std::shared_ptr<audio_io::OutputDeviceFactory> getOutputDeviceFactory() {
	return *audio_output_factory;
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetCount(unsigned int* destination) {
	PUB_BEGIN
	*destination = (*audio_output_factory)->getOutputCount();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetName(unsigned int index, char** destination) {
	PUB_BEGIN
	auto n = (*audio_output_factory)->getOutputNames();
	if(index >= n.size()) ERROR(Lav_ERROR_RANGE, "Invalid device index.");
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
	auto c = (*audio_output_factory)->getOutputMaxChannels();
	if(index >= c.size()) ERROR(Lav_ERROR_RANGE, "Invalid device index.");
	*destination = c[index];
	PUB_END
}

}