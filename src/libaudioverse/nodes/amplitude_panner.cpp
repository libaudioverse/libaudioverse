/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/simulation.hpp>
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

AmplitudePannerNode::AmplitudePannerNode(std::shared_ptr<Simulation> simulation): Node(Lav_OBJTYPE_AMPLITUDE_PANNER_NODE, simulation, 1, 0),
panner(simulation->getBlockSize(), simulation->getSr()) {
	appendInputConnection(0, 1);
	appendOutputConnection(0, 0);
	auto cb = [&](){recomputeChannelMap();};
	getProperty(Lav_PANNER_CHANNEL_MAP).setPostChangedCallback(cb);
}

std::shared_ptr<Node>createAmplitudePannerNode(std::shared_ptr<Simulation> simulation) {
	auto retval = standardNodeCreation<AmplitudePannerNode>(simulation);
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

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createAmplitudePannerNode(simulation);
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