/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <algorithm>


namespace libaudioverse_implementation {

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class SquareNode: public Node {
	public:
	SquareNode(std::shared_ptr<Simulation> simulation);
	void recompute();
	virtual void process();
	float phase = 0.0f, phase_increment=0.0f, on_for = 0.0f, prev_integral=0.0f;
};

SquareNode::SquareNode(std::shared_ptr<Simulation> simulation): Node(Lav_OBJTYPE_SQUARE_NODE, simulation, 0, 1) {
	appendOutputConnection(0, 1);
}

std::shared_ptr<Node> createSquareNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<SquareNode>(simulation);
}

void SquareNode::recompute() {
	float freq= getProperty(Lav_SQUARE_FREQUENCY).getFloatValue();
	float dutyCycle = getProperty(Lav_SQUARE_DUTY_CYCLE).getFloatValue();
	on_for=dutyCycle;
	phase_increment = freq/simulation->getSr();
}

//The following algorithm came from KVR.
//If we consider the square in the continuous domain, we can integrate with the following integral function.
//By averaging adjacent integrals, we obtain an approximation with higher-than-nyquist harmonics lowpassed.

float squareIntegral(float phase, float on_for) {
	return std::min(phase, on_for);
}

void SquareNode::process() {
	recompute();
	//we incorporate this here, the effect is the same.
	float phase_offset =getProperty(Lav_SQUARE_PHASE).getFloatValue();
	for(int i= 0; i < block_size; i++) {
		float currentPhase= phase+phase_offset;
		//we make this wider to further reduce aliasing
		output_buffers[0][i] = (squareIntegral(currentPhase+8*phase_increment, on_for)-squareIntegral(currentPhase, on_for))/(8*phase_increment);
		//That's between 0 and 1, so scale.
		output_buffers[0][i] *= 2;
		output_buffers[0][i] -= 1;
		phase += phase_increment;
		phase -= floorf(phase);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSquareNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createSquareNode(simulation);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}