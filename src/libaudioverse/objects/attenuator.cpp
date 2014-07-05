/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <limits>
#include <memory>

class LavAttenuatorObject: public LavObject {
	public:
	LavAttenuatorObject(std::shared_ptr<LavDevice> device, unsigned int numChannels);
	virtual void process();
};

LavAttenuatorObject::LavAttenuatorObject(std::shared_ptr<LavDevice> device, unsigned int numChannels): LavObject(Lav_OBJTYPE_ATTENUATOR, device, numChannels, numChannels) {
}

std::shared_ptr<LavObject>createAttenuatorObject(std::shared_ptr<LavDevice> device, unsigned int numChannels) {
	auto retval = std::make_shared<LavAttenuatorObject>(device, numChannels);
	device->associateObject(retval);
	return retval;
}

void LavAttenuatorObject::process() {
	const float mul = getProperty(Lav_ATTENUATOR_MULTIPLIER).getFloatValue();
	for(unsigned int i = 0; i < block_size; i++) {
		for(unsigned int j = 0; j < num_outputs; j++) {
			outputs[j][i] = inputs[j][i]*mul;
		}
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createAttenuatorObject(LavDevice* device, unsigned int numChannels, LavObject** destination) {
	PUB_BEGIN
	LOCK(*device);
	auto retval = createAttenuatorObject(device, numChannels);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
