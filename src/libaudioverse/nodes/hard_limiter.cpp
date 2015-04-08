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

class LavHardLimiterNode: public LavNode {
	public:
	LavHardLimiterNode(std::shared_ptr<LavSimulation> simulation, unsigned int numInputs);
	virtual void process();
};

LavHardLimiterNode::LavHardLimiterNode(std::shared_ptr<LavSimulation> simulation, unsigned int numInputs): LavNode(Lav_OBJTYPE_HARD_LIMITER_NODE, simulation, numInputs, numInputs) {
	appendInputConnection(0, numInputs);
	appendOutputConnection(0, numInputs);
}

std::shared_ptr<LavNode>createHardLimiterNode(std::shared_ptr<LavSimulation> simulation, unsigned int numChannels) {
	auto retval = std::shared_ptr<LavHardLimiterNode>(new LavHardLimiterNode(simulation, numChannels), LavObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void LavHardLimiterNode::process() {
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

Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterNode(LavHandle simulationHandle, unsigned int numChannels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createHardLimiterNode(simulation, numChannels);
	*destination = outgoingObject<LavNode>(retval);
	PUB_END
}
