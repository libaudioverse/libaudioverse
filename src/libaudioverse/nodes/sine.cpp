/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/sine.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/implementations/sin_osc.hpp>

namespace libaudioverse_implementation {

SineNode::SineNode(std::shared_ptr<Simulation> simulation): Node(Lav_OBJTYPE_SINE_NODE, simulation, 0, 1), oscillator(simulation->getSr()) {
	appendOutputConnection(0, 1);
}

std::shared_ptr<Node> createSineNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<SineNode>(simulation);
}

void SineNode::process() {
	if(werePropertiesModified(this, Lav_OSCILLATOR_PHASE)) oscillator.setPhase(oscillator.getPhase()+getProperty(Lav_OSCILLATOR_PHASE).getFloatValue());
	auto &freq = getProperty(Lav_OSCILLATOR_FREQUENCY);
	auto &freqMul = getProperty(Lav_OSCILLATOR_FREQUENCY_MULTIPLIER);
	if(freq.needsARate() | freqMul.needsARate()) {
		for(int i=0; i < block_size; i++) {
			oscillator.setFrequency(freq.getFloatValue(i)*freqMul.getFloatValue(i));
			output_buffers[0][i] = oscillator.tick();
		}
	}
	else {
		oscillator.setFrequency(freq.getFloatValue()*freqMul.getFloatValue());
		for(int i = 0; i < block_size; i++) {
			output_buffers[0][i] = (float)oscillator.tick();
		}
	}
}

void SineNode::reset() {
	oscillator.reset();
	oscillator.setPhase(getProperty(Lav_OSCILLATOR_PHASE).getFloatValue());
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createSineNode(simulation);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}