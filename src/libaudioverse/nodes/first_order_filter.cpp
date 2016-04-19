/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/first_order_filter.hpp>
#include <libaudioverse/implementations/first_order_filter.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/multichannel_filter_bank.hpp>
#include <memory>

namespace libaudioverse_implementation {

FirstOrderFilterNode::FirstOrderFilterNode(std::shared_ptr<Simulation> sim, int channels): Node(Lav_OBJTYPE_FIRST_ORDER_FILTER_NODE, sim, channels, channels),
bank(simulation->getSr()) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	bank.setChannelCount(channels);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createFirstOrderFilterNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<FirstOrderFilterNode>(simulation, channels);
}

void FirstOrderFilterNode::process() {
	auto &poleProp = getProperty(Lav_FIRST_ORDER_FILTER_POLE);
	auto &zeroProp = getProperty(Lav_FIRST_ORDER_FILTER_ZERO);
	//Four cases: none is a-rate, pole is a-rate, zero is a-rate, both are a-rate.
	if(zeroProp.needsARate() && poleProp.needsARate()) {
		bank.process(block_size, &input_buffers[0], &output_buffers[0], [&] (FirstOrderFilter& filt, int index) {
			filt.setZeroPosition(zeroProp.getFloatValue(index), false);
			filt.setPolePosition(poleProp.getFloatValue(index), false);
			filt.normalize();
		});
	}
	else if(zeroProp.needsARate()) {
		bank.process(block_size, &input_buffers[0], &output_buffers[0], [&] (FirstOrderFilter& filt, int index) {
			filt.setZeroPosition(zeroProp.getFloatValue(index), false);
			filt.normalize();
		});
	}
	else if(poleProp.needsARate()) {
		bank.process(block_size, &input_buffers[0], &output_buffers[0], [&] (FirstOrderFilter& filt, int index) {
			filt.setPolePosition(poleProp.getFloatValue(index), false);
			filt.normalize();
		});
	}
	else {
		bank->setZeroPosition(zeroProp.getFloatValue(), false);
		bank->setPolePosition(poleProp.getFloatValue(), false);
		bank->normalize();
		bank.process(block_size, &input_buffers[0], &output_buffers[0]);
	}
}

void FirstOrderFilterNode::configureLowpass(float freq) {
	bank->configureLowpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::configureHighpass(float freq) {
	bank->configureHighpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::configureAllpass(float freq) {
	bank->configureAllpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::recomputePoleAndZero() {
	auto& filt = *bank;
	getProperty(Lav_FIRST_ORDER_FILTER_POLE).setFloatValue(filt.getPolePosition());
	getProperty(Lav_FIRST_ORDER_FILTER_ZERO).setFloatValue(filt.getZeroPosition());
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createFirstOrderFilterNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createFirstOrderFilterNode(simulation, channels);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

//I'm not typing this 3 times.
#define conf(which) \
Lav_PUBLIC_FUNCTION LavError Lav_firstOrderFilterNodeConfigure##which(LavHandle nodeHandle, float frequency) {\
	PUB_BEGIN\
	auto n = incomingObject<FirstOrderFilterNode>(nodeHandle);\
	LOCK(*n);\
	n->configure##which(frequency);\
	PUB_END\
}

conf(Lowpass)
conf(Highpass)
conf(Allpass)

}