/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/gain.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

GainNode::GainNode(std::shared_ptr<Simulation> sim): Node(Lav_OBJTYPE_GAIN_NODE, sim, 0, 0) {
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createGainNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<GainNode>(simulation);
}

void GainNode::process() {
	for(unsigned int i = 0; i < input_buffers.size(); i++) {
		std::copy(input_buffers[i], input_buffers[i]+block_size, output_buffers[i]);
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createGainNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createGainNode(simulation);
	retval->resize(channels, channels);
	retval->appendInputConnection(0, channels);
	retval->appendOutputConnection(0, channels);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

}