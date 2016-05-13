/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/split_merge.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>


namespace libaudioverse_implementation {

SplitMergeNode::SplitMergeNode(std::shared_ptr<Server> server, int type): Node(type, server, 0, 1) {
}

std::shared_ptr<Node> createSplitMergeNode(std::shared_ptr<Server> server, int type) {
	return standardNodeCreation<SplitMergeNode>(server, type);
}

std::shared_ptr<Node> createChannelSplitterNode(std::shared_ptr<Server> server, int channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	auto retval= createSplitMergeNode(server, Lav_OBJTYPE_CHANNEL_SPLITTER_NODE);
	retval->resize(channels, 0);
	for(int i =0; i < channels; i++) retval->appendOutputConnection(i, 1);
	retval->appendInputConnection(0, channels);
	server->associateNode(retval);
	return retval;
}

std::shared_ptr<Node> createChannelMergerNode(std::shared_ptr<Server> server, int channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	auto retval = createSplitMergeNode(server, Lav_OBJTYPE_CHANNEL_MERGER_NODE);
	retval->resize(channels, 0);
	retval->appendOutputConnection(0, channels);
	for(int i = 0; i < channels; i++) retval->appendInputConnection(i, 1);
	server->associateNode(retval);
	return retval;
}

int SplitMergeNode::getOutputBufferCount() {
	return getInputBufferCount();
}

float** SplitMergeNode::getOutputBufferArray() {
	return getInputBufferArray();
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createChannelSplitterNode(LavHandle serverHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createChannelSplitterNode(server, channels);
	*destination = outgoingObject(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_createChannelMergerNode(LavHandle serverHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createChannelMergerNode(server, channels);
	*destination = outgoingObject(retval);
	PUB_END
}

}