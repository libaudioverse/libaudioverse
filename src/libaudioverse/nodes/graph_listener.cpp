/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/graph_listener.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <memory>
#include <algorithm>

namespace libaudioverse_implementation {

GraphListenerNode::GraphListenerNode(std::shared_ptr<Server> s, unsigned int channels): Node(Lav_OBJTYPE_GRAPH_LISTENER_NODE, s, channels, channels) {
	outgoing_buffer = allocArray<float>(channels*s->getBlockSize());
	this->channels = channels;
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createGraphListenerNode(std::shared_ptr<Server> server, unsigned int channels) {
	return standardNodeCreation<GraphListenerNode>(server, channels);
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

Lav_PUBLIC_FUNCTION LavError Lav_createGraphListenerNode(LavHandle serverHandle, unsigned int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	*destination = outgoingObject<Node>(createGraphListenerNode(server, channels));
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