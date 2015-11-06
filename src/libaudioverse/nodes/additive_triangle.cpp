/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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
}

std::shared_ptr<Node> createAdditiveTriangleNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<AdditiveTriangleNode>(simulation);
}

void AdditiveTriangleNode::process() {
	if(werePropertiesModified(this, Lav_TRIANGLE_HARMONICS)) oscillator.setHarmonics(getProperty(Lav_TRIANGLE_HARMONICS).getIntValue());
	if(werePropertiesModified(this, Lav_TRIANGLE_PHASE)) oscillator.setPhase(getProperty(Lav_TRIANGLE_PHASE).getFloatValue()+oscillator.getPhase());
	auto &freq = getProperty(Lav_TRIANGLE_FREQUENCY);
	if(freq.needsARate()) {
		for(int i = 0; i < block_size; i++) {
			oscillator.setFrequency(freq.getFloatValue(i));
			output_buffers[0][i] = oscillator.tick();
		}
	}
	else {
		oscillator.setFrequency(freq.getFloatValue());
		for(int i = 0; i < block_size; i++) {
			output_buffers[0][i] = oscillator.tick();
		}
	}
}

void AdditiveTriangleNode::reset() {
	oscillator.reset();
	oscillator.setPhase(getProperty(Lav_TRIANGLE_PHASE).getFloatValue());
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