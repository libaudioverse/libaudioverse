/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/dc_blocker.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/implementations/dc_blocker.hpp>

namespace libaudioverse_implementation {

DcBlockerNode::DcBlockerNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_DC_BLOCKER_NODE, simulation, channels, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Can only filter 1 or greater channels.");
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	this->channels = channels;
	blockers = new DcBlocker*[channels];
	for(int i = 0; i < channels; i++) blockers[i] = new DcBlocker(simulation->getSr());
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createDcBlockerNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<DcBlockerNode>(simulation, channels);
}

DcBlockerNode::~DcBlockerNode() {
	for(int i = 0; i < channels; i++) delete blockers[i];
	delete[] blockers;
}

void DcBlockerNode::process() {
	for(int channel = 0; channel < channels; channel++) {
		auto &b = *blockers[channel];
		for(int i = 0; i < block_size; i++) {
			output_buffers[channel][i] = b.tick(input_buffers[channel][i]);
		}
	}
}

void DcBlockerNode::reset() {
	for(int i = 0; i < channels; i++) blockers[i]->reset();
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createDcBlockerNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createDcBlockerNode(simulation, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}