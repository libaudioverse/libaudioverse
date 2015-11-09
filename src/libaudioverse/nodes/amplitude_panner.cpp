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
#include <libaudioverse/implementations/amplitude_panner.hpp>
#include <libaudioverse/nodes/amplitude_panner.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <memory>

namespace libaudioverse_implementation {

float standard_map_stereo[] = {-90.0f, 90.0f};
float standard_map_40[] = {-45.0, 45.0, -135.0, 135.0};
float standard_map_51[] = {-22.5f, 22.5f, -110.0f, 110.0f};
float standard_map_71[] = {-22.5f, 22.5f, -150.0f, 150.0f, -110.0f, 110.0f};

//This class needs to be public because of the multipanner, which needs to make a method call against it directly.
//see include/libaudioverse/objects/panner.hpp.

AmplitudePannerNode::AmplitudePannerNode(std::shared_ptr<Simulation> simulation): Node(Lav_OBJTYPE_AMPLITUDE_PANNER_NODE, simulation, 1, 0) {
	appendInputConnection(0, 1);
	appendOutputConnection(0, 0);
	auto cb = [&](){recomputeChannelMap();};
	getProperty(Lav_PANNER_CHANNEL_MAP).setPostChangedCallback(cb);
	getProperty(Lav_PANNER_SKIP_CENTER).setPostChangedCallback(cb);
	getProperty(Lav_PANNER_SKIP_LFE).setPostChangedCallback(cb);
	getProperty(Lav_PANNER_HAS_LFE).setPostChangedCallback(cb);
	getProperty(Lav_PANNER_HAS_CENTER).setPostChangedCallback(cb);
}

std::shared_ptr<Node>createAmplitudePannerNode(std::shared_ptr<Simulation> simulation) {
	auto retval = standardNodeCreation<AmplitudePannerNode>(simulation);
	//needed because the inputs/outputs logic needs shared_from_this to be working.
	retval->recomputeChannelMap();
	return retval;
}

void AmplitudePannerNode::recomputeChannelMap() {
	panner.reset();
	skip_lfe = getProperty(Lav_PANNER_SKIP_LFE).getIntValue() == 1;
	skip_center = getProperty(Lav_PANNER_SKIP_CENTER).getIntValue() == 1;
	has_lfe = getProperty(Lav_PANNER_HAS_LFE).getIntValue() == 1;
	has_center = getProperty(Lav_PANNER_HAS_CENTER).getIntValue() == 1;
	Property& channelMap = getProperty(Lav_PANNER_CHANNEL_MAP);
	unsigned int max = channelMap.getFloatArrayLength();
	if(skip_lfe && has_lfe) max++;
	if(skip_center && has_center) max++;
	resize(1, max);
	getOutputConnection(0)->reconfigure(0, max);
	unsigned int index = 0;
	for(unsigned int i = 0; i < max; i++) {
		if(i == 2 && skip_center) continue;
		if(i == 3 && skip_lfe) continue;
		float angle = channelMap.readFloatArray(index);
		panner.addEntry(angle, i);
		index++;
	}
}

void AmplitudePannerNode::process() {
	float azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	float passthrough =getProperty(Lav_PANNER_PASSTHROUGH).getFloatValue();
	panner.pan(azimuth, block_size, input_buffers[0], num_output_buffers, &output_buffers[0]);
	if(passthrough != 0.0f) {
		scalarMultiplicationKernel(block_size, passthrough, input_buffers[0], input_buffers[0]);
		for(int i = 0; i < num_output_buffers; i++) {
			if(i ==2 && skip_center) continue;
			if(i==3 && skip_lfe) continue;
			scalarMultiplicationKernel(block_size, 1.0f-passthrough, output_buffers[i], output_buffers[i]);
			additionKernel(block_size, input_buffers[0], output_buffers[i], output_buffers[i]);
		}
	}
}

void AmplitudePannerNode::configureStandardChannelMap(unsigned int channels) {
	switch(channels) {
		case 2:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(2, standard_map_stereo);
		has_center = false;
		has_lfe = false;
		break;
		case 4:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(4, standard_map_40);
		has_center = false;
		has_lfe = false;
		break;
		case 6:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(4, standard_map_51);
		has_center = true;
		has_lfe = true;
		break;
		case 8:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(6, standard_map_71);
		has_center = true;
		has_lfe = true;
		break;
	};
	getProperty(Lav_PANNER_SKIP_LFE).setIntValue(1);
	getProperty(Lav_PANNER_SKIP_CENTER).setIntValue(1);
	getProperty(Lav_PANNER_HAS_LFE).setIntValue(has_lfe);
	getProperty(Lav_PANNER_HAS_CENTER).setIntValue(has_center);
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