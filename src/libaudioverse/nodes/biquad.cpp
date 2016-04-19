/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/biquad.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/biquad.hpp>
#include <libaudioverse/private/multichannel_filter_bank.hpp>
#include <memory>


namespace libaudioverse_implementation {

BiquadNode::BiquadNode(std::shared_ptr<Simulation> sim, unsigned int channels): Node(Lav_OBJTYPE_BIQUAD_NODE, sim, channels, channels),
bank(simulation->getSr()) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	bank.setChannelCount(channels);
	prev_type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createBiquadNode(std::shared_ptr<Simulation> simulation, unsigned int channels) {
	return standardNodeCreation<BiquadNode>(simulation, channels);
}

void BiquadNode::reconfigure() {
	int type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
	float sr = simulation->getSr();
	float frequency = getProperty(Lav_BIQUAD_FREQUENCY).getFloatValue();
	float q = getProperty(Lav_BIQUAD_Q).getFloatValue();
	float dbgain= getProperty(Lav_BIQUAD_DBGAIN).getFloatValue();
	bank->configure(type, frequency, dbgain, q);
	if(type != prev_type) bank.reset();
	prev_type = type;
}

void BiquadNode::process() {
	if(werePropertiesModified(this, Lav_BIQUAD_FILTER_TYPE, Lav_BIQUAD_DBGAIN, Lav_BIQUAD_FREQUENCY, Lav_BIQUAD_Q)) reconfigure();
	bank.process(block_size, &input_buffers[0], &output_buffers[0]);
}

void BiquadNode::reset() {
	bank.reset();
}

Lav_PUBLIC_FUNCTION LavError Lav_createBiquadNode(LavHandle simulationHandle, unsigned int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createBiquadNode(simulation, channels));
	PUB_END
}

}