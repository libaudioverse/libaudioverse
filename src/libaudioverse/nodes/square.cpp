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
#include <limits>

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class LavSquareNode: public LavNode {
	public:
	LavSquareNode(std::shared_ptr<LavSimulation> simulation);
	void recomputeCounters();
	virtual void process();
	int wave_length, on_for, counter = 0;
};

LavSquareNode::LavSquareNode(std::shared_ptr<LavSimulation> simulation): LavNode(Lav_OBJTYPE_SQUARE_NODE, simulation, 0, 1) {
	getProperty(Lav_SQUARE_FREQUENCY).setPostChangedCallback([=] (){recomputeCounters();});
	getProperty(Lav_SQUARE_DUTY_CYCLE).setPostChangedCallback([=] (){recomputeCounters();});
	recomputeCounters();
	appendOutputConnection(0, 1);
}

std::shared_ptr<LavNode> createSquareNode(std::shared_ptr<LavSimulation> simulation) {
	std::shared_ptr<LavSquareNode> retval = std::shared_ptr<LavSquareNode>(new LavSquareNode(simulation), LavObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void LavSquareNode::recomputeCounters() {
	float freq= getProperty(Lav_SQUARE_FREQUENCY).getFloatValue();
	float dutyCycle = getProperty(Lav_SQUARE_DUTY_CYCLE).getFloatValue();
	wave_length = (int)(simulation->getSr()/freq);
	on_for=(int)(wave_length*dutyCycle);
}

void LavSquareNode::process() {
	for(int i= 0; i < block_size; i++) {
		output_buffers[0][i] = counter < on_for ? 1.0f : 0.0f;
		counter++;
		counter%= wave_length;
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSquareNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createSquareNode(simulation);
	*destination = outgoingObject<LavNode>(retval);
	PUB_END
}
