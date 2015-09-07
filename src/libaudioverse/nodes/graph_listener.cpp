/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <memory>
#include <algorithm>

namespace libaudioverse_implementation {

class GraphListenerNode: public Node {
	public:
	GraphListenerNode(std::shared_ptr<Simulation> sim, unsigned int channels);
	~GraphListenerNode();
	void process();
	LavGraphListenerNodeListeningCallback callback = nullptr;
	float* outgoing_buffer = nullptr;
	void* callback_userdata = nullptr;
	unsigned int channels = 0;
};

GraphListenerNode::GraphListenerNode(std::shared_ptr<Simulation> sim, unsigned int channels): Node(Lav_OBJTYPE_GRAPH_LISTENER_NODE, sim, channels, channels) {
	outgoing_buffer = allocArray<float>(channels*sim->getBlockSize());
	this->channels = channels;
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
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