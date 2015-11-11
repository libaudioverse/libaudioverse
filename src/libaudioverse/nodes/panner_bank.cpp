/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/panner_bank.hpp>
#include <libaudioverse/nodes/gain.hpp>
#include <libaudioverse/nodes/multipanner.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <memory>


namespace libaudioverse_implementation {

PannerBankNode::PannerBankNode(std::shared_ptr<Simulation> sim, int pannerCount, std::shared_ptr<HrtfData> hrtf): SubgraphNode(Lav_OBJTYPE_PANNER_BANK_NODE, sim)  {
	if(pannerCount < 2) ERROR(Lav_ERROR_RANGE, "Must use at least 2 panners.");
	input_gain = createGainNode(simulation);
	output_gain =createGainNode(simulation);
	input_gain->resize(pannerCount, pannerCount);
	for(int i = 0; i < pannerCount; i++) {
		input_gain->appendInputConnection(i, 1);
		input_gain->appendOutputConnection(i, 1);
	}
	output_gain->resize(2, 2);
	output_gain->appendInputConnection(0, 2);
	output_gain->appendOutputConnection(0, 2);
	setInputNode(input_gain);
	setOutputNode(output_gain);
	for(int i = 0; i < pannerCount; i++) {
		panners.push_back(createMultipannerNode(simulation, hrtf));
		input_gain->connect(i, panners[i], 0);
		panners[i]->connect(0, output_gain, 0);
	}
	appendOutputConnection(0, 2);
}

std::shared_ptr<Node> createPannerBankNode(std::shared_ptr<Simulation> simulation, int pannerCount, std::shared_ptr<HrtfData> hrtf) {
	auto retval = standardNodeCreation<PannerBankNode>(simulation, pannerCount, hrtf);
	//this call must be here because it involves shared_from_this.
	retval->configureForwardedProperties();
	simulation->registerNodeForWillTick(retval);
	return retval;
}

void PannerBankNode::configureForwardedProperties() {
	auto us = std::static_pointer_cast<Node>(shared_from_this());
	for(auto &n: panners) {
		//We handle azimuth and elevation.
		n->forwardProperty(Lav_PANNER_SHOULD_CROSSFADE, us, Lav_PANNER_SHOULD_CROSSFADE);
		n->forwardProperty(Lav_PANNER_STRATEGY, us, Lav_PANNER_STRATEGY);
	}
}

void PannerBankNode::strategyChanged() {
	int newStrategy = getProperty(Lav_PANNER_STRATEGY).getIntValue();
	int newChannels =0;
	switch(newStrategy) {
		case Lav_PANNING_STRATEGY_STEREO:
		newChannels=2;
		break;
		case Lav_PANNING_STRATEGY_HRTF:
		newChannels =2;
		break;
		case Lav_PANNING_STRATEGY_SURROUND51:
		newChannels=6;
		break;
		case Lav_PANNING_STRATEGY_SURROUND71:
		newChannels=8;
		break;
		default:
		newChannels=2;
		break;
	}
	output_gain->resize(newChannels, newChannels);
	output_gain->getInputConnection(0)->reconfigure(0, newChannels);
	output_gain->getOutputConnection(0)->reconfigure(0, newChannels);
	getOutputConnection(0)->reconfigure(0, newChannels);
}

void PannerBankNode::needsRepositioning() {
	float azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	int count=panners.size();
	float spread = getProperty(Lav_PANNER_BANK_SPREAD).getFloatValue();
	//We want to hit the endpoints, thus the -1.
	float azimuthDelta=spread/(count-1);
	if(getProperty(Lav_PANNER_BANK_IS_CENTERED).getIntValue() !=0) {
		azimuth -= spread/2.0f;
	}
	for(int i =0; i < count; i++) {
		panners[i]->getProperty(Lav_PANNER_AZIMUTH).setFloatValue(azimuth);
		azimuth += azimuthDelta;
	}
}

void PannerBankNode::willTick() {
	if(werePropertiesModified(this, Lav_PANNER_STRATEGY)) strategyChanged();
	if(werePropertiesModified(this, Lav_PANNER_AZIMUTH, Lav_PANNER_BANK_SPREAD, Lav_PANNER_BANK_IS_CENTERED)) needsRepositioning();
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createPannerBankNode(LavHandle simulationHandle, int pannerCount, char* hrtfPath, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto hrtf = createHrtfFromString(hrtfPath, simulation->getSr());
	*destination = outgoingObject<Node>(createPannerBankNode(simulation, pannerCount, hrtf));
	PUB_END
}

}