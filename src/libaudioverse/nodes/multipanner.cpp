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
#include <libaudioverse/nodes/panner.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/creators.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <utility>
#include <vector>

class LavMultipannerObject: public LavSubgraphNode {
	public:
	LavMultipannerObject(std::shared_ptr<LavSimulation> sim, std::shared_ptr<LavHrtfData> hrtf);
	std::shared_ptr<LavNode> hrtf_panner = nullptr, amplitude_panner = nullptr, input= nullptr;
	void forwardAzimuth();
	void forwardElevation();
	void forwardShouldCrossfade();
	void strategyChanged();
};

LavMultipannerObject::LavMultipannerObject(std::shared_ptr<LavSimulation> sim, std::shared_ptr<LavHrtfData> hrtf): LavSubgraphNode(Lav_NODETYPE_MULTIPANNER, sim)  {
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
	getProperty(Lav_PANNER_AZIMUTH).setPostChangedCallback([this](){forwardAzimuth();});
	getProperty(Lav_PANNER_ELEVATION).setPostChangedCallback([this](){forwardElevation();});
	getProperty(Lav_PANNER_SHOULD_CROSSFADE).setPostChangedCallback([this](){forwardShouldCrossfade();});
	getProperty(Lav_PANNER_STRATEGY).setPostChangedCallback([this](){strategyChanged();});
}

std::shared_ptr<LavNode> createMultipannerNode(std::shared_ptr<LavSimulation> sim, std::shared_ptr<LavHrtfData> hrtf) {
	auto retval = std::shared_ptr<LavMultipannerObject>(new LavMultipannerObject(sim, hrtf), LavObjectDeleter);
	sim->associateNode(retval);
	//this call must be here because it involves shared_from_this.
	retval->strategyChanged();
	return retval;
}

void LavMultipannerObject::forwardAzimuth() {
	float az = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	hrtf_panner->getProperty(Lav_PANNER_AZIMUTH).setFloatValue(az);
	amplitude_panner->getProperty(Lav_PANNER_AZIMUTH).setFloatValue(az);
}

void LavMultipannerObject::forwardElevation() {
	float elev = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	hrtf_panner->getProperty(Lav_PANNER_ELEVATION).setFloatValue(elev);
	amplitude_panner->getProperty(Lav_PANNER_ELEVATION).setFloatValue(elev);
}

void LavMultipannerObject::forwardShouldCrossfade() {
	int cf = getProperty(Lav_PANNER_SHOULD_CROSSFADE).getIntValue();
	hrtf_panner->getProperty(Lav_PANNER_SHOULD_CROSSFADE).setIntValue(cf);
	amplitude_panner->getProperty(Lav_PANNER_SHOULD_CROSSFADE).setIntValue(cf);
}

void LavMultipannerObject::strategyChanged() {
	int newStrategy = getProperty(Lav_PANNER_STRATEGY).getIntValue();
	bool hookHrtf = false, hookAmplitude = false;
	switch(newStrategy) {
		case Lav_PANNING_STRATEGY_HRTF:
		hookHrtf = true;
		break;
		case Lav_PANNING_STRATEGY_STEREO:
		std::dynamic_pointer_cast<LavAmplitudePannerNode>(amplitude_panner)->configureStandardChannelMap(2);
		hookAmplitude = true;
		break;
		case Lav_PANNING_STRATEGY_SURROUND51:
		std::dynamic_pointer_cast<LavAmplitudePannerNode>(amplitude_panner)->configureStandardChannelMap(6);
		hookAmplitude = true;
		break;
		case Lav_PANNING_STRATEGY_SURROUND71:
		std::dynamic_pointer_cast<LavAmplitudePannerNode>(amplitude_panner)->configureStandardChannelMap(8);
		hookAmplitude=true;
		break;
	}
	if(hookAmplitude) {
		int newOutputSize = amplitude_panner->getOutputConnection(0)->getCount();
		getOutputConnection(0)->reconfigure(0, newOutputSize);
		setOutputNode(amplitude_panner);
	}
	if(hookHrtf) {
		getOutputConnection(0)->reconfigure(0, 2);
		setOutputNode(hrtf_panner);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createMultipannerNode(LavHandle simulationHandle, char* hrtfPath, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	std::shared_ptr<LavHrtfData> hrtf = std::make_shared<LavHrtfData>();
	if(std::string(hrtfPath) == "default") {
		hrtf->loadFromDefault(simulation->getSr());
	} else {
		hrtf->loadFromFile(hrtfPath, simulation->getSr());
	}
	*destination = outgoingObject<LavNode>(createMultipannerNode(simulation, hrtf));
	PUB_END
}
