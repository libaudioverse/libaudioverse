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
	LavCustomNode(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int outputs);
	void process();
	LavCustomNodeProcessingCallback callback = nullptr;
	void* callback_userdata = nullptr;
};

LavCustomNode::LavCustomNode(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int outputs): LavNode(Lav_NODETYPE_CUSTOM, sim, inputs, outputs) {
}

std::shared_ptr<LavNode> createCustomNode(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int outputs) {
	auto retval = std::shared_ptr<LavCustomNode>(new LavCustomNode(sim, inputs, outputs), LavNodeDeleter);
	sim->associateNode(retval);
	return retval;
}

void LavCustomNode::process() {
	if(callback == nullptr) {
		LavNode::process();
		return;
	}
	callback(this, block_size, getInputCount(), &inputs[0], getOutputCount(), &outputs[0], callback_userdata);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createCustomNode(LavSimulation* simulation, unsigned int inputs, unsigned int outputs, LavNode** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = outgoingPointer<LavNode>(createCustomNode(incomingPointer<LavSimulation>(simulation), inputs, outputs));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_customNodeSetProcessingCallback(LavNode* node, LavCustomNodeProcessingCallback callback, void* userdata) {
	PUB_BEGIN
	LOCK(*node);
	if(node->getType() != Lav_NODETYPE_CUSTOM) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	LavCustomNode* node2=(LavCustomNode*)node;
	node2->callback = callback;
	node2->callback_userdata = userdata;
	PUB_END
}
