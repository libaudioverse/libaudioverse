/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/resampler.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <utility>
#include <vector>
#include <lambdatask/threadsafe_queue.hpp>

class LavCustomNode: public LavNode {
	public:
	LavCustomNode(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs, unsigned int channelsPerOutput);
	void process();
	LavCustomNodeProcessingCallback callback = nullptr;
	void* callback_userdata = nullptr;
};

LavCustomNode::LavCustomNode(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs, unsigned int channelsPerOutput): LavNode(Lav_NODETYPE_CUSTOM, sim, inputs*channelsPerInput, outputs*channelsPerOutput) {
	for(unsigned int i= 0; i < inputs; i++) appendInputConnection(i*channelsPerInput, channelsPerInput);
	for(int i= 0; i < outputs; i++) appendOutputConnection(i*channelsPerOutput, channelsPerOutput);
}

std::shared_ptr<LavNode> createCustomNode(std::shared_ptr<LavSimulation> simulation, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs,  unsigned int channelsPerOutput) {
	auto retval = std::shared_ptr<LavCustomNode>(new LavCustomNode(simulation, inputs, channelsPerInput, outputs, channelsPerOutput), LavObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void LavCustomNode::process() {
	if(callback != nullptr) {
		callback(externalObjectHandle, block_size, num_input_buffers, getInputBufferArray(), num_output_buffers, getOutputBufferArray(), callback_userdata);
	}
	else {
		for(int i= 0; i < num_output_buffers; i++) std::copy(input_buffers[i], input_buffers[i]+block_size, output_buffers[i]);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createCustomNode(LavHandle simulationHandle, unsigned int inputs, unsigned int channelsPerInput, unsigned int outputs, unsigned int channelsPerOutput, LavHandle* destination) {
	PUB_BEGIN
	auto simulation=incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<LavNode>(createCustomNode(simulation, inputs, channelsPerInput, outputs, channelsPerOutput));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_customNodeSetProcessingCallback(LavHandle nodeHandle, LavCustomNodeProcessingCallback callback, void* userdata) {
	PUB_BEGIN
	auto node= incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_NODETYPE_CUSTOM) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	auto node2 = std::static_pointer_cast<LavCustomNode>(node);
	node2->callback = callback;
	node2->callback_userdata = userdata;
	PUB_END
}
