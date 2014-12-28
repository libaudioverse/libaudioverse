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

class LavRingmodObject: public LavObject {
	public:
	LavRingmodObject(std::shared_ptr<LavSimulation> sim);
	void process();
};

LavRingmodObject::LavRingmodObject(std::shared_ptr<LavSimulation> sim): LavObject(Lav_OBJTYPE_PULL, sim, 2, 1) {
}

std::shared_ptr<LavObject> createRingmodObject(std::shared_ptr<LavSimulation> sim) {
	auto retval = std::make_shared<LavRingmodObject>(sim);
	sim->associateObject(retval);
	return retval;
}

void LavRingmodObject::process() {
	for(unsigned int i = 0; i < block_size; i++) outputs[0][i]=inputs[0][i]*inputs[1][i];
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createRingmodObject(LavSimulation* sim, LavObject** destination) {
	PUB_BEGIN
	LOCK(*sim);
	*destination = outgoingPointer<LavObject>(createRingmodObject(incomingPointer<LavSimulation>(sim)));
	PUB_END
}
