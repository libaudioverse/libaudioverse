/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/buffer_timeline.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/server.hpp>
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

BufferTimelineNode::BufferTimelineNode(std::shared_ptr<Server> server, int channels): Node(Lav_OBJTYPE_BUFFER_TIMELINE_NODE, server, 0, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	appendOutputConnection(0, channels);
	output_channels= channels;
	for(int i = 0; i < channels; i++) workspace.push_back(allocArray<float>(server->getBlockSize()));
}

std::shared_ptr<Node> createBufferTimelineNode(std::shared_ptr<Server> server, int channels) {
	return standardNodeCreation<BufferTimelineNode>(server, channels);
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
	time+=block_size/server->getSr();
}

void BufferTimelineNode::scheduleBuffer(double time, float delta, std::shared_ptr<Buffer> buffer) {
	time+=this->time; //time is relative to the node's internal time.
	//The buffer player handles the buffer's use count.
	auto player = new BufferPlayer(server->getBlockSize(), server->getSr());
	player->setBuffer(buffer);
	player->setRate(delta);
	scheduled_buffers.insert(decltype(scheduled_buffers)::value_type(time, player));
}

void BufferTimelineNode::reset() {
	for(auto &i: scheduled_buffers) delete i.second;
	scheduled_buffers.clear();
}

//begin public API.

Lav_PUBLIC_FUNCTION LavError Lav_createBufferTimelineNode(LavHandle serverHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto s = incomingObject<Server>(serverHandle);
	LOCK(*s);
	if(channels == 0) ERROR(Lav_ERROR_RANGE);
	auto n =createBufferTimelineNode(s, channels);
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