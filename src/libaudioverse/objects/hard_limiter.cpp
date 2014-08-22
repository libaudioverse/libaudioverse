/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <memory>

class LavHardLimiterObject: public LavObject {
	public:
	LavHardLimiterObject(std::shared_ptr<LavSimulation> simulation, unsigned int numInputs);
	virtual void process();
};

LavHardLimiterObject::LavHardLimiterObject(std::shared_ptr<LavSimulation> simulation, unsigned int numInputs): LavObject(Lav_OBJTYPE_HARD_LIMITER, simulation, numInputs, numInputs) {
}

std::shared_ptr<LavObject>createHardLimiterObject(std::shared_ptr<LavSimulation> simulation, unsigned int numChannels) {
	auto retval = std::make_shared<LavHardLimiterObject>(simulation, numChannels);
	simulation->associateObject(retval);
	return retval;
}

void LavHardLimiterObject::process() {
	for(unsigned int i = 0; i < block_size; i++) {
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

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterObject(LavSimulation* simulation, unsigned int numChannels, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createHardLimiterObject(incomingPointer<LavSimulation>(simulation), numChannels);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
