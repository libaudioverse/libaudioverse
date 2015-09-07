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
#include <memory>
#include <algorithm>

namespace libaudioverse_implementation {

class CustomNode: public Node {
	public:
	CustomNode(std::shared_ptr<Simulation> sim, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs, unsigned int channelsPerOutput);
	void process();
	LavCustomNodeProcessingCallback callback = nullptr;
	void* callback_userdata = nullptr;
};

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