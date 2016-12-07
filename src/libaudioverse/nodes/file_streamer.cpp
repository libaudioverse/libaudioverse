/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/file_streamer.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <string>
#include <memory>
#include <math.h>

namespace libaudioverse_implementation {

FileStreamerNode::FileStreamerNode(std::shared_ptr<Server> server, std::string path): Node(Lav_OBJTYPE_FILE_STREAMER_NODE, server, 0, 1),
streamer(path, server->getBlockSize(), server->getSr()) {
	resize(0, streamer.getChannels());
	appendOutputConnection(0, streamer.getChannels());
	getProperty(Lav_FILE_STREAMER_POSITION).setDoubleRange(0.0, streamer.getDuration());
	end_callback = std::make_shared<Callback<void()>>();
}

std::shared_ptr<Node> createFileStreamerNode(std::shared_ptr<Server> server, std::string path) {
	return standardNodeCreation<FileStreamerNode>(server, path);
}

void FileStreamerNode::process() {
	if(werePropertiesModified(this, Lav_FILE_STREAMER_POSITION)) streamer.setPosition(getProperty(Lav_FILE_STREAMER_POSITION).getDoubleValue());
	if(werePropertiesModified(this, Lav_FILE_STREAMER_LOOPING)) streamer.setIsLooping(getProperty(Lav_FILE_STREAMER_LOOPING).getIntValue() != 0);
	streamer.process(&output_buffers[0]);
	getProperty(Lav_FILE_STREAMER_POSITION).setDoubleValue(streamer.getPosition());
	if(streamer.getEnded()) {
		getProperty(Lav_FILE_STREAMER_ENDED).setIntValue(1);
		server->enqueueTask([=] () {(*end_callback)();});
	}
	else getProperty(Lav_FILE_STREAMER_ENDED).setIntValue(0);
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createFileStreamerNode(LavHandle serverHandle, const char* path, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createFileStreamerNode(server, std::string(path));
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_fileStreamerNodeSetEndCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata) {
	PUB_BEGIN
	auto n = incomingObject<FileStreamerNode>(nodeHandle);
	if(callback) {
		n->end_callback->setCallback(wrapParameterlessCallback(n, callback, userdata));
	}
	else n->end_callback->clear();
	PUB_END
}

}