/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/allpass.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/multichannel_filter_bank.hpp>
#include <libaudioverse/implementations/allpass.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

AllpassNode::AllpassNode(std::shared_ptr<Simulation> sim, int channels, int maxDelay): Node(Lav_OBJTYPE_ALLPASS_NODE, sim, channels, channels),
//The +1 here deals with any floating point inaccuracies.
bank((maxDelay+1)/simulation->getSr(), simulation->getSr()) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	if(maxDelay < 1) ERROR(Lav_ERROR_RANGE, "You need to allow for at least 1 sample of delay.");
	getProperty(Lav_ALLPASS_DELAY_SAMPLES_MAX).setIntValue(maxDelay);
	getProperty(Lav_ALLPASS_DELAY_SAMPLES).setIntRange(0, maxDelay);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	bank.setChannelCount(channels);
}

std::shared_ptr<Node> createAllpassNode(std::shared_ptr<Simulation> simulation, int channels, int maxDelay) {
	return standardNodeCreation<AllpassNode>(simulation, channels, maxDelay);
}

void AllpassNode::reconfigureCoefficient() {
	float c = getProperty(Lav_ALLPASS_COEFFICIENT).getFloatValue();
	bank->setCoefficient(c);
}

void AllpassNode::reconfigureDelay() {
	int d = getProperty(Lav_ALLPASS_DELAY_SAMPLES).getIntValue();
	bank->line.setDelayInSamples(d);
}

void AllpassNode::reconfigureInterpolationTime() {
	float it = getProperty(Lav_ALLPASS_INTERPOLATION_TIME).getFloatValue();
	bank->line.setInterpolationTime(it);
}

void AllpassNode::process() {
	if(werePropertiesModified(this, Lav_ALLPASS_INTERPOLATION_TIME)) reconfigureInterpolationTime();
	if(werePropertiesModified(this, Lav_ALLPASS_COEFFICIENT)) reconfigureCoefficient();
	if(werePropertiesModified(this, Lav_ALLPASS_DELAY_SAMPLES)) reconfigureDelay();
	bank.process(block_size, &input_buffers[0], &output_buffers[0]);
}

void AllpassNode::reset() {
	bank.reset();
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createAllpassNode(LavHandle simulationHandle, int channels, int maxDelay, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createAllpassNode(simulation, channels, maxDelay);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

}