/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/file_streamer.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <string>
#include <memory>
#include <math.h>

namespace libaudioverse_implementation {

FileStreamerNode::FileStreamerNode(std::shared_ptr<Simulation> simulation, std::string path): Node(Lav_OBJTYPE_FILE_STREAMER_NODE, simulation, 0, 1),
streamer(path, simulation->getBlockSize(), simulation->getSr()) {
	resize(0, streamer.getChannels());
	appendOutputConnection(0, streamer.getChannels());
	getProperty(Lav_FILE_STREAMER_POSITION).setDoubleRange(0.0, streamer.getDuration());
	end_callback = std::make_shared<Callback<void()>>();
}

std::shared_ptr<Node> createFileStreamerNode(std::shared_ptr<Simulation> simulation, std::string path) {
	return standardNodeCreation<FileStreamerNode>(simulation, path);
}

void FileStreamerNode::process() {
	if(werePropertiesModified(this, Lav_FILE_STREAMER_POSITION)) streamer.setPosition(getProperty(Lav_FILE_STREAMER_POSITION).getDoubleValue());
	if(werePropertiesModified(this, Lav_FILE_STREAMER_LOOPING)) streamer.setIsLooping(getProperty(Lav_FILE_STREAMER_LOOPING).getIntValue() != 0);
	streamer.process(&output_buffers[0]);
	getProperty(Lav_FILE_STREAMER_POSITION).setDoubleValue(streamer.getPosition());
	if(streamer.getEnded()) {
		getProperty(Lav_FILE_STREAMER_ENDED).setIntValue(1);
		simulation->enqueueTask([=] () {(*end_callback)();});
	}
	else getProperty(Lav_FILE_STREAMER_ENDED).setIntValue(0);
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createFileStreamerNode(LavHandle simulationHandle, const char* path, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createFileStreamerNode(simulation, std::string(path));
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