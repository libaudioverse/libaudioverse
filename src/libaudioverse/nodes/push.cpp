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

class PushNode: public Node {
	public:
	PushNode(std::shared_ptr<Simulation> sim, unsigned int inputSr, unsigned int channels);
	~PushNode();
	void process();
	void feed(unsigned int length, float* buffer);
	unsigned int input_sr = 0;
	std::shared_ptr<speex_resampler_cpp::Resampler> resampler = nullptr;
	float* workspace = nullptr;
	//the push_* variables are for the public api to feed us.
	float* push_buffer = nullptr;
	unsigned int push_channels = 0;
	unsigned int push_frames = 1024;
	unsigned int push_offset = 0;
	bool fired_out_callback = false;
};

PushNode::PushNode(std::shared_ptr<Simulation> sim, unsigned int inputSr, unsigned int channels): Node(Lav_OBJTYPE_PUSH_NODE, sim, 0, channels) {
	if(channels == 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	input_sr = inputSr;
	resampler = speex_resampler_cpp::createResampler(push_frames, channels, inputSr, (int)sim->getSr());
	this->push_channels = channels;
	workspace = allocArray<float>(push_channels*simulation->getBlockSize());
	push_buffer = allocArray<float>(push_frames*channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createPushNode(std::shared_ptr<Simulation> simulation, unsigned int inputSr, unsigned int channels) {
	return standardNodeCreation<PushNode>(simulation, inputSr, channels);
}

PushNode::~PushNode() {
	freeArray(workspace);
	freeArray(push_buffer);
}

void PushNode::process() {
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

void PushNode::feed(unsigned int length, float* buffer) {
	fired_out_callback = false;
	if(length%push_channels != 0) ERROR(Lav_ERROR_RANGE, "Length must be a multiple of the configured channels.");
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
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createPushNode(simulation, sr, channels));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_pushNodeFeed(LavHandle nodeHandle, unsigned int length, float* buffer) {
	PUB_BEGIN
	auto node=incomingObject<Node>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_PUSH_NODE) ERROR(Lav_ERROR_TYPE_MISMATCH, "Expected a push node.");
	std::static_pointer_cast<PushNode>(node)->feed(length, buffer);
	PUB_END
}

}