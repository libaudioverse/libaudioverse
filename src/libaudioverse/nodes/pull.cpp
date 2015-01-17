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

class LavPullObject: public LavNode {
	public:
	LavPullObject(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels);
	~LavPullObject();
	void process();
	unsigned int input_sr = 0, channels = 0;
	std::shared_ptr<LavResampler> resampler = nullptr;
	float* incoming_buffer = nullptr, *resampled_buffer = nullptr;
	LavPullNodeAudioCallback callback = nullptr;
	void* callback_userdata = nullptr;
};

LavPullObject::LavPullObject(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels): LavNode(Lav_NODETYPE_PULL, sim, 0, channels) {
	this->channels = channels;
	input_sr = inputSr;
	resampler = std::make_shared<LavResampler>(sim->getBlockSize(), channels, inputSr, (int)sim->getSr());
	this->channels = channels;
	incoming_buffer = LavAllocFloatArray(channels*simulation->getBlockSize());
	resampled_buffer = LavAllocFloatArray(channels*sim->getBlockSize());
}

std::shared_ptr<LavNode> createPullNode(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels) {
	auto retval = std::shared_ptr<LavPullObject>(new LavPullObject(sim, inputSr, channels), LavNodeDeleter);
	sim->associateNode(retval);
	return retval;
}

LavPullObject::~LavPullObject() {
	LavFreeFloatArray(incoming_buffer);
	LavFreeFloatArray(resampled_buffer);
}

void LavPullObject::process() {
	//first get audio into the resampler if needed.
	unsigned int got = 0;
	while(got < block_size) {
		got += resampler->write(resampled_buffer, block_size-got);
		if(got >= block_size) break; //we may have done it on this iteration.
		if(callback) {
			callback(this, block_size, channels, incoming_buffer, callback_userdata);
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

Lav_PUBLIC_FUNCTION LavError Lav_createPullNode(LavSimulation* simulation, unsigned int sr, unsigned int channels, LavNode** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = outgoingPointer<LavNode>(createPullNode(incomingPointer<LavSimulation>(simulation), sr, channels));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_pullNodeSetAudioCallback(LavNode* node, LavPullNodeAudioCallback callback, void* userdata) {
	PUB_BEGIN
	LOCK(*node);
	if(node->getType() != Lav_NODETYPE_PULL) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	((LavPullObject*)node)->callback = callback;
	((LavPullObject*)node)->callback_userdata = userdata;
	PUB_END
}
