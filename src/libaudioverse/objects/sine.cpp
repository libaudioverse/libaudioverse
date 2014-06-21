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
#include <limits>

class LavSineObject: public LavObject {
	public:
	virtual void init(LavDevice* dev);
	virtual void process();
	float table_delta;
	unsigned int start ;
	float offset;
};

void LavSineObject::init(LavDevice* dev) {
	LavObject::init(dev, 0, 1);
	table_delta = sineTableLength/device->getSr();
	start = 0;
	offset = 0;
	properties[Lav_SINE_FREQUENCY] = createFloatProperty("frequency", 440.0f, 0.0f, std::numeric_limits<float>::infinity());
}

LavObject* createSineObject(LavDevice* device) {
	LavSineObject* retval = new LavSineObject();
	retval->init(device);
	return retval;
}

void LavSineObject::process() {
	float freq = properties[Lav_SINE_FREQUENCY]->getFloatValue();
	for(unsigned int i = 0; i < device->getBlockSize(); i++) {
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
	LavObject* retval = createSineObject(device);
	*destination = retval;
	PUB_END
}