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

class LavPushNode: public LavNode {
	public:
	LavPushNode(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels);
	~LavPushNode();
	void process();
	void feed(unsigned int length, float* buffer);
	unsigned int input_sr = 0;
	std::shared_ptr<LavResampler> resampler = nullptr;
	float* workspace = nullptr;
	//the push_* variables are for the public api to feed us.
	float* push_buffer = nullptr;
	unsigned int push_channels = 0;
	unsigned int push_frames = 1024;
	unsigned int push_offset = 0;
	bool fired_out_callback = false;
};

LavPushNode::LavPushNode(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels): LavNode(Lav_OBJTYPE_PUSH_NODE, sim, 0, channels) {
	input_sr = inputSr;
	resampler = std::make_shared<LavResampler>(push_frames, channels, inputSr, (int)sim->getSr());
	this->push_channels = channels;
	workspace = LavAllocArray<float>(push_channels*simulation->getBlockSize());
	push_buffer = LavAllocArray<float>(push_frames*channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<LavNode> createPushNode(std::shared_ptr<LavSimulation> simulation, unsigned int inputSr, unsigned int channels) {
	auto retval = std::shared_ptr<LavPushNode>(new LavPushNode(simulation, inputSr, channels), LavObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

LavPushNode::~LavPushNode() {
	LavFreeArray(workspace);
	LavFreeArray(push_buffer);
}

void LavPushNode::process() {
	memset(workspace, 0, sizeof(float)*push_channels*block_size);
	unsigned int got = resampler->write(workspace, simulation->getBlockSize());
	if(got < simulation->getBlockSize()) {
		resampler->read(push_buffer);
		memset(push_buffer, 0, sizeof(float)*push_channels*push_frames);
		push_offset = 0;
		resampler->write(workspace+got*push_channels, simulation->getBlockSize()-got);
		if(fired_out_callback == false) {
			getEvent(Lav_PUSH_OUT_EVENT).fire();
			fired_out_callback = true;
		}
	}
	for(unsigned int i = 0; i < push_channels*block_size; i++) {
		unsigned int output = i%push_channels;
		unsigned int position = i/push_channels;
		output_buffers[output][position] = workspace[i];
	}
	float threshold = getProperty(Lav_PUSH_THRESHOLD).getFloatValue();
	float remaining = resampler->estimateAvailableFrames()/(float)simulation->getSr();
	if(remaining < threshold) {
		getEvent(Lav_PUSH_AUDIO_EVENT).fire();
	}
}

void LavPushNode::feed(unsigned int length, float* buffer) {
	fired_out_callback = false;
	if(length%push_channels != 0) throw LavErrorException(Lav_ERROR_RANGE);
	unsigned int frames = length/push_channels;
	unsigned int offset = 0;
	while(offset < frames) {
		if(push_offset == push_frames) {
			resampler->read(push_buffer);
			push_offset = 0;
			memset(push_buffer, 0, sizeof(float)*push_frames*push_channels);
		}
		for(unsigned int i = 0; i < push_channels; i++) push_buffer[push_offset*push_channels+i] = buffer[offset*push_channels+i];
		offset ++;
		push_offset ++;
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createPushNode(LavHandle simulationHandle, unsigned int sr, unsigned int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<LavNode>(createPushNode(simulation, sr, channels));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_pushNodeFeed(LavHandle nodeHandle, unsigned int length, float* buffer) {
	PUB_BEGIN
	auto node=incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_PUSH_NODE) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	std::static_pointer_cast<LavPushNode>(node)->feed(length, buffer);
	PUB_END
}
