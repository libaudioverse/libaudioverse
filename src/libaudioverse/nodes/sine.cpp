/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/functiontables.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <limits>

namespace libaudioverse_implementation {

class SineNode: public Node {
	public:
	SineNode(std::shared_ptr<Simulation> simulation);
	virtual void process();
	virtual void reset() override;
	float phase = 0;
};

SineNode::SineNode(std::shared_ptr<Simulation> simulation): Node(Lav_OBJTYPE_SINE_NODE, simulation, 0, 1) {
	appendOutputConnection(0, 1);
}

std::shared_ptr<Node> createSineNode(std::shared_ptr<Simulation> simulation) {
	std::shared_ptr<SineNode> retval = std::shared_ptr<SineNode>(new SineNode(simulation), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void SineNode::process() {
	auto &freqProp = getProperty(Lav_SINE_FREQUENCY);
	if(freqProp.needsARate()==false) {
		float phaseDelta=freqProp.getFloatValue()/simulation->getSr();
		for(unsigned int i = 0; i< block_size; i++) {
			//The (float) here keeps us from converting to double because of PI.
			output_buffers[0][i] = (float)sinf(2*phase*(float)PI);
			phase+=phaseDelta;
		}
	}
	else {
		for(unsigned int i = 0; i< block_size; i++) {
			float phaseDelta=freqProp.getFloatValue(i)/simulation->getSr();
			output_buffers[0][i] = (float)sin(2*phase*PI);
			phase+=phaseDelta;
		}
	}
	phase -=floor(phase);
}

void SineNode::reset() {
	phase=0.0;
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