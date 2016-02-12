/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/implementations/multipanner.hpp>
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

//We always have 8 channels for simplicity.
MultipannerNode::MultipannerNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf): Node(Lav_OBJTYPE_MULTIPANNER_NODE, simulation, 1, 8),
panner(simulation->getBlockSize(), simulation->getSr(), hrtf) {
	appendInputConnection(0, 1);
	appendOutputConnection(0, 2);
	strategyChanged();
}

std::shared_ptr<Node> createMultipannerNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf) {
	auto retval = standardNodeCreation<MultipannerNode>(simulation, hrtf);
	retval->strategyChanged();
	simulation->registerNodeForWillTick(retval);
	return retval;
}

void MultipannerNode::strategyChanged() {
	int newStrategy = getProperty(Lav_PANNER_STRATEGY).getIntValue();
	int channels=2;
	switch(newStrategy) {
		case Lav_PANNING_STRATEGY_HRTF:
		case Lav_PANNING_STRATEGY_STEREO:
		channels = 2;
		break;
		case Lav_PANNING_STRATEGY_SURROUND40:
		channels = 4;
		break;
		case Lav_PANNING_STRATEGY_SURROUND51:
		channels=6;
		break;
		case Lav_PANNING_STRATEGY_SURROUND71:
		channels =8;
		break;
	}
	getOutputConnection(0)->reconfigure(0, channels);
}

void MultipannerNode::process() {
	if(werePropertiesModified(this, Lav_PANNER_STRATEGY)) strategyChanged();
	if(werePropertiesModified(this, Lav_PANNER_AZIMUTH)) panner.setAzimuth(getProperty(Lav_PANNER_AZIMUTH).getFloatValue());
	if(werePropertiesModified(this, Lav_PANNER_ELEVATION)) panner.setElevation(getProperty(Lav_PANNER_ELEVATION).getFloatValue());
	if(werePropertiesModified(this, Lav_PANNER_SHOULD_CROSSFADE)) panner.setShouldCrossfade(getProperty(Lav_PANNER_SHOULD_CROSSFADE).getIntValue() == 1);
	if(werePropertiesModified(this, Lav_PANNER_STRATEGY)) panner.setStrategy(getProperty(Lav_PANNER_STRATEGY).getIntValue());
	panner.process(input_buffers[0], &output_buffers[0]);
}

void MultipannerNode::reset() {
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