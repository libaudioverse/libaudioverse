/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <memory>

namespace libaudioverse_implementation {

class HardLimiterNode: public Node {
	public:
	HardLimiterNode(std::shared_ptr<Simulation> simulation, int channels);
	virtual void process();
};

HardLimiterNode::HardLimiterNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_HARD_LIMITER_NODE, simulation, channels, channels) {
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node>createHardLimiterNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<HardLimiterNode>(simulation, channels);
}

void HardLimiterNode::process() {
	for(unsigned int i = 0; i < block_size; i++) {
		for(unsigned int o = 0; o < num_output_buffers; o++) {
			if(input_buffers[o][i] > 1.0f) {
				output_buffers[o][i] = 1.0f;
				continue;
			}
			else if(input_buffers[o][i] < -1.0f) {
				output_buffers[o][i] = -1.0f;
				continue;
			}
			output_buffers[o][i] = input_buffers[o][i];
		}
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createHardLimiterNode(simulation, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}