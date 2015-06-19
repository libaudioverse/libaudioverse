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
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/feedback_delay_network.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <limits>
#include <algorithm>
#include <random>

namespace libaudioverse_implementation {

//Delay line offsets as percents.
const float delay_percents[16] = {
0.0, -0.21, 0.12, -0.13,
0.256, -0.349, 0.513, -0.515,
0.07, -0.09, 0.011, 0.032,
0.43, 0.47, 0.23, 0.19,
};

class LateReflectionsNode: public Node {
	public:
	LateReflectionsNode(std::shared_ptr<Simulation> simulation);
	~LateReflectionsNode();
	virtual void process();
	void recompute();
	FeedbackDelayNetwork fdn;
	float* delays = nullptr;
	float* gains = nullptr;
	float* output_frame=nullptr;
	float* normalized_hadamard = nullptr;
};

LateReflectionsNode::LateReflectionsNode(std::shared_ptr<Simulation> simulation):
Node(Lav_OBJTYPE_LATE_REFLECTIONS_NODE, simulation, 16, 16),
fdn(16, (1.0/50.0)*16.0, simulation->getSr()) {
	for(int i=0; i < 16; i++) {
		appendInputConnection(i, 1);
		appendOutputConnection(i, 1);
	}
	normalized_hadamard=allocArray<float>(16*16);
	//normalize the hadamard.
	float norm = 1.0f/sqrtf(16);
	for(int i = 0; i < 16*16; i++) normalized_hadamard[i] = norm*hadamard16[i];
	//feed the fdn the initial matrix.
	fdn.setMatrix(normalized_hadamard);
	//this is fixed...for now.
	fdn.setDelayCrossfadingTime(0.1);
	gains = allocArray<float>(16);
	output_frame=allocArray<float>(16);
	delays=allocArray<float>(16);
	//property callbacks.
	getProperty(Lav_LATE_REFLECTIONS_T60).setPostChangedCallback([=] () {recompute();});
	getProperty(Lav_LATE_REFLECTIONS_DENSITY).setPostChangedCallback([=](){recompute();});
	recompute();
}

std::shared_ptr<Node> createLateReflectionsNode(std::shared_ptr<Simulation> simulation) {
	std::shared_ptr<LateReflectionsNode> retval = std::shared_ptr<LateReflectionsNode>(new LateReflectionsNode(simulation), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

LateReflectionsNode::~LateReflectionsNode() {
	freeArray(gains);
	freeArray(output_frame);
	freeArray(delays);
}

void LateReflectionsNode::recompute() {
	float density = getProperty(Lav_LATE_REFLECTIONS_DENSITY).getFloatValue();
	//We see 16 times as many reflections as we expect because there are 16 inputs and outputs.
	//Put another way, each delay line is contributing, on average, density reflections.
	density /= 16.0;
	float t60=getProperty(Lav_LATE_REFLECTIONS_T60).getFloatValue();
	//The base delay is the amount we are delaying all delay lines by.
	float base_delay=1.0f/density;
	for(int i= 0; i < 16; i++) {
		//percent increase/decrease: p=new/old-1
		//new from old: new=(p+1)*old
		delays[i] = (delay_percents[i]+1)*base_delay;
	}
	fdn.setDelays(delays);
	//Amount of db decrease in one second.
	double dbPerSec=-60.0/t60;
	for(int i= 0; i < 16; i++) {
		//Db decrease for one circulation of the delay line.
		double dbPerPeriod = dbPerSec*delays[i];
		//convert to a gain.
		gains[i] = pow(10, dbPerPeriod/20.0);
	}
}

void LateReflectionsNode::process() {
	for(int i= 0; i < block_size; i++) {
		//Get the fdn's output.
		fdn.computeFrame(output_frame);
		for(int j= 0; j < 16; j++) output_buffers[j][i] = output_frame[j];
		multiplicationKernel(16, gains, output_frame, output_frame);
		//bring in the inputs.
		for(int j = 0; j < 16; j++) output_frame[j] += input_buffers[j][i];
		fdn.advance(output_frame);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createLateReflectionsNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createLateReflectionsNode(simulation);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}