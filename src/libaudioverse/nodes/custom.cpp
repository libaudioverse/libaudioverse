/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/custom.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <memory>
#include <algorithm>

namespace libaudioverse_implementation {

CustomNode::CustomNode(std::shared_ptr<Simulation> sim, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs, unsigned int channelsPerOutput): Node(Lav_OBJTYPE_CUSTOM_NODE, sim, inputs*channelsPerInput, outputs*channelsPerOutput) {
	for(unsigned int i= 0; i < inputs; i++) appendInputConnection(i*channelsPerInput, channelsPerInput);
	for(int i= 0; i < outputs; i++) appendOutputConnection(i*channelsPerOutput, channelsPerOutput);
}

std::shared_ptr<Node> createCustomNode(std::shared_ptr<Simulation> simulation, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs,  unsigned int channelsPerOutput) {
	return standardNodeCreation<CustomNode>(simulation, inputs, channelsPerInput, outputs, channelsPerOutput);
}

void CustomNode::process() {
	if(callback != nullptr) {
		callback(outgoingObject(this->shared_from_this()), block_size, num_input_buffers, getInputBufferArray(), num_output_buffers, getOutputBufferArray(), callback_userdata);
	}
	else {
		for(int i= 0; i < num_output_buffers; i++) std::copy(input_buffers[i], input_buffers[i]+block_size, output_buffers[i]);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createCustomNode(LavHandle simulationHandle, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs, unsigned int channelsPerOutput, LavHandle* destination) {
	PUB_BEGIN
	auto simulation=incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createCustomNode(simulation, inputs, channelsPerInput, outputs, channelsPerOutput));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_customNodeSetProcessingCallback(LavHandle nodeHandle, LavCustomNodeProcessingCallback callback, void* userdata) {
	PUB_BEGIN
	auto node= incomingObject<Node>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_CUSTOM_NODE) ERROR(Lav_ERROR_TYPE_MISMATCH, "Expected a custom node.");
	auto node2 = std::static_pointer_cast<CustomNode>(node);
	node2->callback = callback;
	node2->callback_userdata = userdata;
	PUB_END
}

}