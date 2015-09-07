/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/buffer.hpp>
#include <limits>
#include <iterator>
#include <vector>

namespace libaudioverse_implementation {

class ScheduledBuffer {
	public:
	ScheduledBuffer(std::shared_ptr<Buffer> buffer, double time, float delta, int outputChannels);
	bool add(float* destination, int maxFrames, int channel); //returns true when there is no more to write.
	std::shared_ptr<Buffer> buffer;
	int length = 0, output_channels = 0, buffer_channels = 0;
	std::vector<int> position;
	float offset = 0.0f;
	double time;
	float delta = 1.0;
};

ScheduledBuffer::ScheduledBuffer(std::shared_ptr<Buffer> buffer, double time, float delta, int outputChannels) {
	this->buffer=buffer;
	this->output_channels=outputChannels;
	this->time = time;
	this->length=buffer->getLength();
	this->buffer_channels = buffer->getChannels();
	this->delta = delta;
	this->position.resize(output_channels, 0);
}

bool ScheduledBuffer::add(float* destination, int maxFrames, int channel) {
	//linear interpolation with mixing matrix application.
	//this should really be optimized.
	float s1, s2, w1, w2;
	int i1, i2;
	//guard against zero-length buffers:
	if(length== 0) return true;
	for(int frame = 0; frame < maxFrames; frame++) {
		i1 = position[channel];
		i2=std::min(position[channel]+1, length-1);
		w1 = 1-offset;
		w2 = offset;
		s1 = buffer->getSampleWithMixingMatrix(i1, channel, output_channels);
		s2 = buffer->getSampleWithMixingMatrix(i2, channel, output_channels);
		destination[frame] += s1*w1+s2*w2;
		offset+=delta;
		position[channel] += (int)floorf(offset);
		offset-=floorf(offset);
		if(position[channel] == length) return true;
	}
	return false;
}

//So we can compare them for insertion purposes.
bool operator<(const ScheduledBuffer &a, const ScheduledBuffer &b) {
	return a.time < b.time;
}

class BufferTimelineNode: public Node {
	public:
	BufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels);
	void process() override;
	void scheduleBuffer(double time, float delta, std::shared_ptr<Buffer> buffer);
	void reset() override;
	private:
	std::vector<ScheduledBuffer> scheduled_buffers;
	std::vector<ScheduledBuffer> active_buffers;
	double time = 0.0;
	int output_channels = 0;
};

BufferTimelineNode::BufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_BUFFER_TIMELINE_NODE, simulation, 0, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	appendOutputConnection(0, channels);
	output_channels= channels;
}

std::shared_ptr<Node> createBufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<BufferTimelineNode>(simulation, channels);
}

void BufferTimelineNode::process() {
	for(auto i=scheduled_buffers.begin(); i !=scheduled_buffers.end(); i++) {
		if(i->time > time+(1.0f/simulation->getSr())*block_size) break; //the queue is sorted, we do want break.
		active_buffers.push_back(*i);
		i = scheduled_buffers.erase(i);
		//We can invalidate the loop and need to check heree.
		if(i== scheduled_buffers.end()) break; //may be because of the delete.
	}
	for(auto i = active_buffers.begin(); i!=active_buffers.end(); i++) {
		bool done = false;
		int offset = 0;
		if(i->time > time) offset = std::min<int>((i->time-time)*simulation->getSr(), block_size);
		int writing = block_size-offset;
		if(writing== 0) continue;
		for(int j = 0; j < num_output_buffers; j++) {
			done = i->add(output_buffers[j]+offset, writing, j);
		}
		if(done) {
			i= active_buffers.erase(i);
			if(i ==active_buffers.end()) break;
		}
	}
	time+=block_size/simulation->getSr();
}

void BufferTimelineNode::scheduleBuffer(double time, float delta, std::shared_ptr<Buffer> buffer) {
	time+=this->time; //time is relative to the node's internal time.
	auto sb=ScheduledBuffer(buffer, time, delta, output_channels);
	auto insertBefore=std::upper_bound(scheduled_buffers.begin(), scheduled_buffers.end(), sb);
	scheduled_buffers.insert(insertBefore, sb);
}

void BufferTimelineNode::reset() {
	scheduled_buffers.clear();
	active_buffers.clear();
}

//begin public API.

Lav_PUBLIC_FUNCTION LavError Lav_createBufferTimelineNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto sim = incomingObject<Simulation>(simulationHandle);
	LOCK(*sim);
	if(channels == 0) ERROR(Lav_ERROR_RANGE);
	auto n =createBufferTimelineNode(sim, channels);
	*destination = outgoingObject(n);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferTimelineNodeScheduleBuffer(LavHandle nodeHandle, LavHandle bufferHandle, double time, float pitchBend) {
	PUB_BEGIN
	auto n = incomingObject<BufferTimelineNode>(nodeHandle);
	auto b = incomingObject<Buffer>(bufferHandle);
	if(n->getType() !=Lav_OBJTYPE_BUFFER_TIMELINE_NODE || b->getType() !=Lav_OBJTYPE_BUFFER) ERROR(Lav_ERROR_TYPE_MISMATCH);
	if(time < 0.0) ERROR(Lav_ERROR_RANGE);
	if(pitchBend <0.0) ERROR(Lav_ERROR_RANGE);
	LOCK(*n);
	n->scheduleBuffer(time, pitchBend, b);
	PUB_END
}

}