/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/implementations/amplitude_panner.hpp>
#include <libaudioverse/nodes/amplitude_panner.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <memory>

namespace libaudioverse_implementation {

AmplitudePannerNode::AmplitudePannerNode(std::shared_ptr<Server> server): Node(Lav_OBJTYPE_AMPLITUDE_PANNER_NODE, server, 1, 0),
panner(server->getBlockSize(), server->getSr()) {
	appendInputConnection(0, 1);
	appendOutputConnection(0, 0);
	auto cb = [&](){recomputeChannelMap();};
	getProperty(Lav_PANNER_CHANNEL_MAP).setPostChangedCallback(cb);
}

std::shared_ptr<Node>createAmplitudePannerNode(std::shared_ptr<Server> server) {
	auto retval = standardNodeCreation<AmplitudePannerNode>(server);
	//needed because the inputs/outputs logic needs shared_from_this to be working.
	retval->recomputeChannelMap();
	return retval;
}

void AmplitudePannerNode::recomputeChannelMap() {
	Property& channelMap = getProperty(Lav_PANNER_CHANNEL_MAP);
	unsigned int newSize = channelMap.getFloatArrayLength();
	resize(1, newSize);
	getOutputConnection(0)->reconfigure(0, newSize);
	panner.readMap(newSize, channelMap.getFloatArrayPtr());
}

void AmplitudePannerNode::process() {
	if(werePropertiesModified(this, Lav_PANNER_AZIMUTH)) panner.setAzimuth(getProperty(Lav_PANNER_AZIMUTH).getFloatValue());
	if(werePropertiesModified(this, Lav_PANNER_ELEVATION)) panner.setElevation(getProperty(Lav_PANNER_ELEVATION).getFloatValue());
	panner.pan(input_buffers[0], &output_buffers[0]);
}

void AmplitudePannerNode::configureStandardChannelMap(int channels) {
	switch(channels) {
		case 2:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(2, standard_panning_map_stereo);
		break;
		case 4:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(4, standard_panning_map_surround40);
		break;
		case 6:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(4, standard_panning_map_surround51);
		break;
		case 8:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(6, standard_panning_map_surround71);
		break;
	};
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerNode(LavHandle serverHandle, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createAmplitudePannerNode(server);
	*destination = outgoingObject(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_amplitudePannerNodeConfigureStandardMap(LavHandle nodeHandle, unsigned int channels) {
	PUB_BEGIN
	if(channels != 2 && channels != 6 && channels != 8) ERROR(Lav_ERROR_RANGE);
	auto node= incomingObject<Node>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_AMPLITUDE_PANNER_NODE) ERROR(Lav_ERROR_TYPE_MISMATCH);
	std::static_pointer_cast<AmplitudePannerNode>(node)->configureStandardChannelMap(channels);
	PUB_END
}

}