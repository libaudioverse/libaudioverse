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
#include <libaudioverse/nodes/buffer.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/buffer.hpp>
#include <limits>
#include <vector>
#include <memory>

namespace libaudioverse_implementation {

BufferNode::BufferNode(std::shared_ptr<Server> server): Node(Lav_OBJTYPE_BUFFER_NODE, server, 0, 1),
player(server->getBlockSize(), server->getSr()) {
	appendOutputConnection(0, 1);
	getProperty(Lav_BUFFER_BUFFER).setPostChangedCallback([&] () {bufferChanged();});
	end_callback = std::make_shared<Callback<void()>>();
}

std::shared_ptr<Node> createBufferNode(std::shared_ptr<Server> server) {
	return standardNodeCreation<BufferNode>(server);
}

void BufferNode::bufferChanged() {
	auto buff = getProperty(Lav_BUFFER_BUFFER).getBufferValue();
	double maxPosition = 0.0;
	int newChannels= 0;
	if(buff==nullptr) {
		resize(0, 1);
		getOutputConnection(0)->reconfigure(0, 1);
	}
	else {
		newChannels = buff->getChannels() > 0 ? buff->getChannels() : 1;
		resize(0, newChannels);
		getOutputConnection(0)->reconfigure(0, newChannels);
		maxPosition =buff->getDuration();
	}
	player.setBuffer(buff);
	getProperty(Lav_BUFFER_POSITION).setDoubleValue(0.0); //the callback handles changing everything else.
	getProperty(Lav_BUFFER_POSITION).setDoubleRange(0.0, maxPosition);
}

void BufferNode::positionChanged() {
	player.setPosition(getProperty(Lav_BUFFER_POSITION).getDoubleValue());
}

void BufferNode::process() {
	auto buff = getProperty(Lav_BUFFER_BUFFER).getBufferValue();
	auto endedCount = getProperty(Lav_BUFFER_ENDED_COUNT).getIntValue();
	if(buff == nullptr) return;
	if(werePropertiesModified(this, Lav_BUFFER_POSITION)) player.setPosition(getProperty(Lav_BUFFER_POSITION).getDoubleValue());
	if(werePropertiesModified(this, Lav_BUFFER_RATE)) player.setRate(getProperty(Lav_BUFFER_RATE).getDoubleValue());
	if(werePropertiesModified(this, Lav_BUFFER_LOOPING)) player.setIsLooping(getProperty(Lav_BUFFER_LOOPING).getIntValue() != 0);
	player.setEndedCount(endedCount);
	int prevEndedCount = player.getEndedCount();
	player.process(buff->getChannels(), &output_buffers[0]);
	// Temporary cludge: the buffer player can return a position past the end of the buffer.
	auto &pos = getProperty(Lav_BUFFER_POSITION);
	pos.setDoubleValue(std::min(player.getPosition(), pos.getDoubleMax()));
	for(int i = player.getEndedCount(); i > prevEndedCount; i--) {
		server->enqueueTask([=] () {(*end_callback)();});
	}
	getProperty(Lav_BUFFER_ENDED_COUNT).setIntValue(player.getEndedCount());
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createBufferNode(LavHandle serverHandle, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createBufferNode(server);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_bufferNodeSetEndCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata) {
	PUB_BEGIN
	auto n = incomingObject<BufferNode>(nodeHandle);
	if(callback) {
		n->end_callback->setCallback(wrapParameterlessCallback(n, callback, userdata));
	}
	else n->end_callback->clear();
	PUB_END
}

}