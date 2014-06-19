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
	table_delta = device->getSr()/sineTableLength;
	start = 0;
	offset = 0;
	properties[Lav_SINE_FREQUENCY] = createFloatProperty("frequency", 440.0f, 0.0f, std::numeric_limits<float>::infinity());
}

void LavSineObject::process() {
}
