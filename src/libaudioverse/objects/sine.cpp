/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_functiontables.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <limits>

class LavSineObject: public LavObject {
	public:
	LavSineObject(std::shared_ptr<LavDevice> device);
	virtual void process();
	float table_delta;
	unsigned int start ;
	float offset;
};

LavSineObject::LavSineObject(std::shared_ptr<LavDevice> device): LavObject(Lav_OBJTYPE_SINE, device, 0, 1) {
	table_delta = sineTableLength/device->getSr();
	start = 0;
	offset = 0;
}

std::shared_ptr<LavObject> createSineObject(std::shared_ptr<LavDevice> device) {
	std::shared_ptr<LavSineObject> retval = std::make_shared<LavSineObject>(device);
	device->associateObject(retval);
	return retval;
}

void LavSineObject::process() {
	float freq = getProperty(Lav_SINE_FREQUENCY).getFloatValue();
	for(unsigned int i = 0; i< block_size; i++) {
		const unsigned int samp1 = start;
		const unsigned int samp2 = start+1;
		const float weight1 = offset;
		const float weight2 = 1-offset;
		outputs[0][i] = sineTable[samp1]*weight1+sineTable[samp2]*weight2;
		offset += table_delta*freq;
		start += (int)floorf(offset);
		start %= sineTableLength;
		offset = fmod(offset, 1.0f);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSineObject(LavDevice* device, LavObject **destination) {
	PUB_BEGIN
	LOCK(*device);
	auto retval = createSineObject(incomingPointer<LavDevice>(device));
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}