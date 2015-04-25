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

class LavSineNode: public LavNode {
	public:
	LavSineNode(std::shared_ptr<LavSimulation> simulation);
	virtual void process();
	virtual void reset() override;
	double phase = 0;
};

LavSineNode::LavSineNode(std::shared_ptr<LavSimulation> simulation): LavNode(Lav_OBJTYPE_SINE_NODE, simulation, 0, 1) {
	appendOutputConnection(0, 1);
}

std::shared_ptr<LavNode> createSineNode(std::shared_ptr<LavSimulation> simulation) {
	std::shared_ptr<LavSineNode> retval = std::shared_ptr<LavSineNode>(new LavSineNode(simulation), LavObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void LavSineNode::process() {
	auto &freqProp = getProperty(Lav_SINE_FREQUENCY);
	if(freqProp.needsARate()==false) {
		double phaseDelta=freqProp.getFloatValue()/simulation->getSr();
		for(unsigned int i = 0; i< block_size; i++) {
			output_buffers[0][i] = (float)sin(2*phase*PI);
			phase+=phaseDelta;
		}
	}
	else {
		for(unsigned int i = 0; i< block_size; i++) {
			float phaseDelta=freqProp.getFloatValue(i)/simulation->getSr();
			output_buffers[0][i] = sin(2*phase*PI);
			phase+=phaseDelta;
		}
	}
	phase -=floorf(phase);
}

void LavSineNode::reset() {
	phase=0.0;
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createSineNode(simulation);
	*destination = outgoingObject<LavNode>(retval);
	PUB_END
}