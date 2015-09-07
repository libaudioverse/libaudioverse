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
#include <speex_resampler_cpp.hpp>
#include <memory>

namespace libaudioverse_implementation {

class PullNode: public Node {
	public:
	PullNode(std::shared_ptr<Simulation> sim, unsigned int inputSr, unsigned int channels);
	~PullNode();
	void process();
	unsigned int input_sr = 0, channels = 0;
	std::shared_ptr<speex_resampler_cpp::Resampler> resampler = nullptr;
	float* incoming_buffer = nullptr, *resampled_buffer = nullptr;
	LavPullNodeAudioCallback callback = nullptr;
	void* callback_userdata = nullptr;
};

PullNode::PullNode(std::shared_ptr<Simulation> sim, unsigned int inputSr, unsigned int channels): Node(Lav_OBJTYPE_PULL_NODE, sim, 0, channels) {
	this->channels = channels;
	input_sr = inputSr;
	resampler = speex_resampler_cpp::createResampler(sim->getBlockSize(), channels, inputSr, (int)sim->getSr());
	this->channels = channels;
	incoming_buffer = allocArray<float>(channels*simulation->getBlockSize());
	resampled_buffer = allocArray<float>(channels*sim->getBlockSize());
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createPullNode(std::shared_ptr<Simulation> simulation, unsigned int inputSr, unsigned int channels) {
	return standardNodeCreation<PullNode>(simulation, inputSr, channels);
}

PullNode::~PullNode() {
	freeArray(incoming_buffer);
	freeArray(resampled_buffer);
}

void PullNode::process() {
	//first get audio into the resampler if needed.
	int got = 0;
	while(got < block_size) {
		got += resampler->write(resampled_buffer, block_size-got);
		if(got >= block_size) break; //we may have done it on this iteration.
		if(callback) {
			callback(outgoingObject(this->shared_from_this()), block_size, channels, incoming_buffer, callback_userdata);
		} else {
			memset(incoming_buffer, 0, block_size*sizeof(float)*channels);
		}
		resampler->read(incoming_buffer);
	}
	//this is simply uninterweaving, but taking advantage of the fact that we have a different output destination.
	for(unsigned int i = 0; i < block_size*channels; i+=channels) {
		for(unsigned int j = 0; j < channels; j++) {
			output_buffers[j][i/channels] = resampled_buffer[i+j];
		}
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createPullNode(LavHandle simulationHandle, unsigned int sr, unsigned int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createPullNode(simulation, sr, channels));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_pullNodeSetAudioCallback(LavHandle nodeHandle, LavPullNodeAudioCallback callback, void* userdata) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_PULL_NODE) ERROR(Lav_ERROR_TYPE_MISMATCH, "Expected a pull node.");
	auto p = std::static_pointer_cast<PullNode>(node);
	p->callback = callback;
	p->callback_userdata = userdata;
	PUB_END
}

}