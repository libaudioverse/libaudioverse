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

class LavGraphListenerNode: public LavNode {
	public:
	LavGraphListenerNode(std::shared_ptr<LavSimulation> sim, unsigned int channels);
	~LavGraphListenerNode();
	void process();
	LavGraphListenerNodeListeningCallback callback = nullptr;
	float* outgoing_buffer = nullptr;
	void* callback_userdata = nullptr;
	unsigned int channels = 0;
};

LavGraphListenerNode::LavGraphListenerNode(std::shared_ptr<LavSimulation> sim, unsigned int channels): LavNode(Lav_NODETYPE_GRAPH_LISTENER, sim, channels, channels) {
	outgoing_buffer = LavAllocFloatArray(channels*sim->getBlockSize());
	this->channels = channels;
}

std::shared_ptr<LavNode> createGraphListenerNode(std::shared_ptr<LavSimulation> sim, unsigned int channels) {
	auto retval = std::shared_ptr<LavGraphListenerNode>(new LavGraphListenerNode(sim, channels), LavNodeDeleter);
	sim->associateNode(retval);
	return retval;
}

LavGraphListenerNode::~LavGraphListenerNode() {
	LavFreeFloatArray(outgoing_buffer);
}

void LavGraphListenerNode::process() {
	if(callback == nullptr) return;
	for(unsigned int i = 0; i < block_size; i++) {
		for(unsigned int j = 0; j < channels; j++) {
			outgoing_buffer[i*channels+j] = inputs[j][i];
			outputs[j][i] = inputs[j][i];
		}
	}
	callback(this, block_size, channels, outgoing_buffer, callback_userdata);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createGraphListenerNode(LavSimulation* simulation, unsigned int channels, LavNode** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = outgoingPointer<LavNode>(createGraphListenerNode(incomingPointer<LavSimulation>(simulation), channels));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_graphListenerNodeSetListeningCallback(LavNode* node, LavGraphListenerNodeListeningCallback callback, void* userdata) {
	PUB_BEGIN
	LOCK(*node);
	if(node->getType() != Lav_NODETYPE_GRAPH_LISTENER) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	LavGraphListenerNode* node2=(LavGraphListenerNode*)node;
	node2->callback = callback;
	node2->callback_userdata = userdata;
	PUB_END
}
