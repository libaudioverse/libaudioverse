/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/hard_limiter.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <memory>

namespace libaudioverse_implementation {

HardLimiterNode::HardLimiterNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_HARD_LIMITER_NODE, simulation, channels, channels) {
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
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