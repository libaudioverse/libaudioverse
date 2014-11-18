/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_resampler.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_kernels.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <utility>
#include <vector>
#include <lambdatask/threadsafe_queue.hpp>

class LavGraphListenerObject: public LavAnalyzerObject {
	public:
	LavGraphListenerObject(std::shared_ptr<LavSimulation> sim, unsigned int channels);
	~LavGraphListenerObject();
	void process();
	LavGraphListenerObjectListeningCallback callback = nullptr;
	float* outgoing_buffer = nullptr;
	void* callback_userdata = nullptr;
	unsigned int channels = 0;
};

LavGraphListenerObject::LavGraphListenerObject(std::shared_ptr<LavSimulation> sim, unsigned int channels): LavAnalyzerObject(Lav_OBJTYPE_GRAPH_LISTENER, sim, channels) {
	outgoing_buffer = new float[channels*sim->getBlockSize()]();
	this->channels = channels;
}

std::shared_ptr<LavObject> createGraphListenerObject(std::shared_ptr<LavSimulation> sim, unsigned int channels) {
	return std::make_shared<LavGraphListenerObject>(sim, channels);
}

LavGraphListenerObject::~LavGraphListenerObject() {
	delete[] outgoing_buffer;
}

void LavGraphListenerObject::process() {
	if(callback == nullptr) return;
	for(unsigned int i = 0; i < block_size*channels; i+=channels) {
		for(unsigned int j = 0; j < channels; j++) {
			outgoing_buffer[i+j] = inputs[j][i];
		}
	}
	callback(this, block_size, channels, outgoing_buffer, callback_userdata);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createGraphListenerObject(LavSimulation* simulation, unsigned int channels, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = outgoingPointer<LavObject>(createGraphListenerObject(incomingPointer<LavSimulation>(simulation), channels));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_graphListenerObjectSetListeningCallback(LavObject* obj, LavGraphListenerObjectListeningCallback callback, void* userdata) {
	PUB_BEGIN
	LOCK(*obj);
	if(obj->getType() != Lav_OBJTYPE_GRAPH_LISTENER) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	LavGraphListenerObject* obj2=(LavGraphListenerObject*)obj;
	obj2->callback = callback;
	obj2->callback_userdata = userdata;
	PUB_END
}
