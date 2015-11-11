/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/multipanner.hpp>
#include <libaudioverse/nodes/amplitude_panner.hpp>
#include <libaudioverse/nodes/hrtf.hpp>
#include <libaudioverse/nodes/gain.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <memory>


namespace libaudioverse_implementation {

MultipannerNode::MultipannerNode(std::shared_ptr<Simulation> sim, std::shared_ptr<HrtfData> hrtf): SubgraphNode(Lav_OBJTYPE_MULTIPANNER_NODE, sim)  {
	hrtf_panner = createHrtfNode(sim, hrtf);
	amplitude_panner = createAmplitudePannerNode(sim);
	input=createGainNode(simulation);
	input->resize(1, 1);
	input->appendInputConnection(0, 1);
	input->appendOutputConnection(0, 1);
	input->connect(0, hrtf_panner, 0);
	input->connect(0, amplitude_panner, 0);
	appendOutputConnection(0, 0);
	setInputNode(input);
}

std::shared_ptr<Node> createMultipannerNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf) {
	auto retval = standardNodeCreation<MultipannerNode>(simulation, hrtf);
	//this call must be here because it involves shared_from_this.
	retval->configureForwardedProperties();
	retval->strategyChanged();
	simulation->registerNodeForWillTick(retval);
	return retval;
}

void MultipannerNode::configureForwardedProperties() {
	auto us = std::static_pointer_cast<Node>(shared_from_this());
	//forward everything common from both, and hrtf stuff from hrtf.
	amplitude_panner->forwardProperty(Lav_PANNER_AZIMUTH, us, Lav_PANNER_AZIMUTH);
	amplitude_panner->forwardProperty(Lav_PANNER_ELEVATION, us, Lav_PANNER_ELEVATION);
	hrtf_panner->forwardProperty(Lav_PANNER_AZIMUTH, us, Lav_PANNER_AZIMUTH);
	hrtf_panner->forwardProperty(Lav_PANNER_ELEVATION, us, Lav_PANNER_ELEVATION);
	//crossfading.
	amplitude_panner->forwardProperty(Lav_PANNER_SHOULD_CROSSFADE, us, Lav_PANNER_SHOULD_CROSSFADE);
	hrtf_panner->forwardProperty(Lav_PANNER_SHOULD_CROSSFADE, us, Lav_PANNER_SHOULD_CROSSFADE);
	//strategy is already only us.
}

void MultipannerNode::strategyChanged() {
	int newStrategy = getProperty(Lav_PANNER_STRATEGY).getIntValue();
	int newOutputSize=2;
	bool hookHrtf = false, hookAmplitude = false;
	switch(newStrategy) {
		case Lav_PANNING_STRATEGY_HRTF:
		hookHrtf = true;
		break;
		case Lav_PANNING_STRATEGY_STEREO:
		std::dynamic_pointer_cast<AmplitudePannerNode>(amplitude_panner)->configureStandardChannelMap(2);
		hookAmplitude = true;
		break;
		case Lav_PANNING_STRATEGY_SURROUND40:
		std::dynamic_pointer_cast<AmplitudePannerNode>(amplitude_panner)->configureStandardChannelMap(4);
		hookAmplitude = true;
		newOutputSize = 4;
		break;
		case Lav_PANNING_STRATEGY_SURROUND51:
		std::dynamic_pointer_cast<AmplitudePannerNode>(amplitude_panner)->configureStandardChannelMap(6);
		hookAmplitude = true;
		newOutputSize=6;
		break;
		case Lav_PANNING_STRATEGY_SURROUND71:
		std::dynamic_pointer_cast<AmplitudePannerNode>(amplitude_panner)->configureStandardChannelMap(8);
		hookAmplitude=true;
		newOutputSize=8;
		break;
	}
	if(hookAmplitude) {
		getOutputConnection(0)->reconfigure(0, newOutputSize);
		setOutputNode(amplitude_panner);
		current_panner = amplitude_panner;
	}
	if(hookHrtf) {
		getOutputConnection(0)->reconfigure(0, 2);
		setOutputNode(hrtf_panner);
		current_panner = hrtf_panner;
	}
}

void MultipannerNode::willTick() {
	if(werePropertiesModified(this, Lav_PANNER_STRATEGY)) strategyChanged();
}

void MultipannerNode::reset() {
	hrtf_panner->reset();
	amplitude_panner->reset();
	input->reset();
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createMultipannerNode(LavHandle simulationHandle, char* hrtfPath, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto hrtf = createHrtfFromString(hrtfPath, simulation->getSr());
	*destination = outgoingObject<Node>(createMultipannerNode(simulation, hrtf));
	PUB_END
}

}