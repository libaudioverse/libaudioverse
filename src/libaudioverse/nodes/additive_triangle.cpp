/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/additive_triangle.hpp>
#include <libaudioverse/implementations/additive_square.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <memory>

namespace libaudioverse_implementation {

AdditiveTriangleNode::AdditiveTriangleNode(std::shared_ptr<Simulation> simulation): Node(Lav_OBJTYPE_ADDITIVE_TRIANGLE_NODE, simulation, 0, 1), oscillator(simulation->getSr()) {
	appendOutputConnection(0, 1);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createAdditiveTriangleNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<AdditiveTriangleNode>(simulation);
}

void AdditiveTriangleNode::process() {
	if(werePropertiesModified(this, Lav_TRIANGLE_HARMONICS)) oscillator.setHarmonics(getProperty(Lav_TRIANGLE_HARMONICS).getIntValue());
	if(werePropertiesModified(this, Lav_OSCILLATOR_PHASE)) oscillator.setPhase(getProperty(Lav_OSCILLATOR_PHASE).getFloatValue()+oscillator.getPhase());
	auto &freq = getProperty(Lav_OSCILLATOR_FREQUENCY);
	auto &freqMul = getProperty(Lav_OSCILLATOR_FREQUENCY_MULTIPLIER);
	if(freq.needsARate() || freqMul.needsARate()) {
		for(int i = 0; i < block_size; i++) {
			oscillator.setFrequency(freq.getFloatValue(i)*freqMul.getFloatValue(i));
			output_buffers[0][i] = oscillator.tick();
		}
	}
	else {
		oscillator.setFrequency(freq.getFloatValue()*freqMul.getFloatValue());
		for(int i = 0; i < block_size; i++) {
			output_buffers[0][i] = oscillator.tick();
		}
	}
}

void AdditiveTriangleNode::reset() {
	oscillator.reset();
	oscillator.setPhase(getProperty(Lav_OSCILLATOR_PHASE).getFloatValue());
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createAdditiveTriangleNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createAdditiveTriangleNode(simulation);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}