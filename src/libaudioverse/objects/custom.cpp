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

class LavCustomObject: public LavObject {
	public:
	LavCustomObject(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int outputs);
	void process();
	LavCustomObjectProcessingCallback callback = nullptr;
	void* callback_userdata = nullptr;
};

LavCustomObject::LavCustomObject(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int outputs): LavObject(Lav_OBJTYPE_CUSTOM, sim, inputs, outputs) {
}

std::shared_ptr<LavObject> createCustomObject(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int outputs) {
	auto retval = std::shared_ptr<LavCustomObject>(new LavCustomObject(sim, inputs, outputs), LavObjectDeleter);
	sim->associateObject(retval);
	return retval;
}

void LavCustomObject::process() {
	if(callback == nullptr) {
		LavObject::process();
		return;
	}
	callback(this, block_size, getInputCount(), &inputs[0], getOutputCount(), &outputs[0], callback_userdata);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createCustomObject(LavSimulation* simulation, unsigned int inputs, unsigned int outputs, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = outgoingPointer<LavObject>(createCustomObject(incomingPointer<LavSimulation>(simulation), inputs, outputs));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_customObjectSetProcessingCallback(LavObject* obj, LavCustomObjectProcessingCallback callback, void* userdata) {
	PUB_BEGIN
	LOCK(*obj);
	if(obj->getType() != Lav_OBJTYPE_CUSTOM) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	LavCustomObject* obj2=(LavCustomObject*)obj;
	obj2->callback = callback;
	obj2->callback_userdata = userdata;
	PUB_END
}
