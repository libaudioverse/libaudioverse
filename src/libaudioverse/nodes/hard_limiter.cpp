/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_devices.hpp>

class LavHardLimiter: public LavObject {
	public:
	void init(LavDevice* device, unsigned int numInputs);
	virtual void process();
};

void LavHardLimiter::init(LavDevice* device, unsigned int numInputs) {
	LavObject::init(device, numInputs, numInputs);
}

void LavHardLimiter::process() {
	for(unsigned int i = 0; i < device->getBlockSize(); i++) {
		for(unsigned int o = 0; o < num_outputs; o++) {
			if(inputs[o][i] > 1.0f) {
				outputs[o][i] = 1.0f;
				continue;
			}
			else if(inputs[o][i] < -1.0f) {
				outputs[o][i] = -1.0f;
				continue;
			}
			outputs[o][i] = inputs[o][i];
		}
	}
}
