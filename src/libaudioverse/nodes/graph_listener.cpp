/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/graph_listener.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <memory>
#include <algorithm>

namespace libaudioverse_implementation {

GraphListenerNode::GraphListenerNode(std::shared_ptr<Simulation> sim, unsigned int channels): Node(Lav_OBJTYPE_GRAPH_LISTENER_NODE, sim, channels, channels) {
	outgoing_buffer = allocArray<float>(channels*sim->getBlockSize());
	this->channels = channels;
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createGraphListenerNode(std::shared_ptr<Simulation> simulation, unsigned int channels) {
	return standardNodeCreation<GraphListenerNode>(simulation, channels);
}

GraphListenerNode::~GraphListenerNode() {
	freeArray(outgoing_buffer);
}

void GraphListenerNode::process() {
	if(callback) {
		for(int i = 0; i < block_size; i++) {
			for(unsigned int j = 0; j < channels; j++) {
				outgoing_buffer[i*channels+j] = input_buffers[j][i];
			}
		}
		callback(outgoingObject(this->shared_from_this()), block_size, channels, outgoing_buffer, callback_userdata);
	}
	for(int i= 0; i < num_output_buffers; i++) std::copy(input_buffers[i], input_buffers[i]+block_size, output_buffers[i]);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createGraphListenerNode(LavHandle simulationHandle, unsigned int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createGraphListenerNode(simulation, channels));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_graphListenerNodeSetListeningCallback(LavHandle nodeHandle, LavGraphListenerNodeListeningCallback callback, void* userdata) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_GRAPH_LISTENER_NODE) ERROR(Lav_ERROR_TYPE_MISMATCH, "Expected a graph listener.");
	auto node2= std::static_pointer_cast<GraphListenerNode>(node);
	node2->callback = callback;
	node2->callback_userdata = userdata;
	PUB_END
}

}