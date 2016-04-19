/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/ringmod.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <memory>

namespace libaudioverse_implementation {

RingmodNode::RingmodNode(std::shared_ptr<Simulation> sim): Node(Lav_OBJTYPE_RINGMOD_NODE, sim, 2, 1) {
	appendInputConnection(0, 1);
	appendInputConnection(1, 1);
	appendOutputConnection(0, 1);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createRingmodNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<RingmodNode>(simulation);
}

void RingmodNode::process() {
	multiplicationKernel(block_size, input_buffers[0], input_buffers[1], output_buffers[0]);
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createRingmodNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createRingmodNode(simulation));
	PUB_END
}

}