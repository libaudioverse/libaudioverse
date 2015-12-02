/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/buffer_timeline.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/buffer.hpp>
#include <libaudioverse/private/helper_templates.hpp>
#include <libaudioverse/implementations/buffer_player.hpp>
#include <iterator>
#include <vector>

namespace libaudioverse_implementation {

BufferTimelineNode::BufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_BUFFER_TIMELINE_NODE, simulation, 0, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	appendOutputConnection(0, channels);
	output_channels= channels;
	for(int i = 0; i < channels; i++) workspace.push_back(allocArray<float>(simulation->getBlockSize()));
}

std::shared_ptr<Node> createBufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<BufferTimelineNode>(simulation, channels);
}

BufferTimelineNode::~BufferTimelineNode() {
	for(auto &i: workspace) freeArray(i);
	for(auto &i: scheduled_buffers) delete i.second;
}

void BufferTimelineNode::process() {
	filter(scheduled_buffers, [&](auto &val)->bool {
		if(val.first > time) return true;
		if(val.second->getEndedCount()) {
			delete val.second;
			return false;
		}
		//Otherwise, add and keep.
		val.second->process(output_channels, &workspace[0]);
		for(int i = 0; i < output_channels; i++) additionKernel(block_size, workspace[i], output_buffers[i], output_buffers[i]);
		return true;
	});
	time+=block_size/simulation->getSr();
}

void BufferTimelineNode::scheduleBuffer(double time, float delta, std::shared_ptr<Buffer> buffer) {
	time+=this->time; //time is relative to the node's internal time.
	//The buffer player handles the buffer's use count.
	auto player = new BufferPlayer(simulation->getBlockSize(), simulation->getSr());
	player->setBuffer(buffer);
	player->setRate(delta);
	scheduled_buffers.insert(decltype(scheduled_buffers)::value_type(time, player));
}

void BufferTimelineNode::reset() {
	for(auto &i: scheduled_buffers) delete i.second;
	scheduled_buffers.clear();
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