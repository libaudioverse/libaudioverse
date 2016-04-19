/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/noise.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/iir.hpp>
#include <random>

namespace libaudioverse_implementation {

//we give the random number generator a fixed seed for debugging purposes.
//Normal distribution with standard deviation 0.25, more than 99% of values are less than 1.
NoiseNode::NoiseNode(std::shared_ptr<Simulation> simulation): Node(Lav_OBJTYPE_NOISE_NODE, simulation, 0, 1),
random_number_generator(1234), normal_distribution(0.0f, 0.25f),
pinkifier(simulation->getSr()),
brownifier(simulation->getSr()) {
	/**We have to configure the pinkifier.
This was originally taken from Spectral Audio processing by JOS.*/
	//zeros
	double pinkNumer[] = {0.049922035, -0.095993537, 0.050612699, -0.004408786};
	//and the poles.
	double pinkDenom[] = {1, -2.494956002,   2.017265875,  -0.522189400};
	pinkifier.configure(sizeof(pinkNumer)/sizeof(double), pinkNumer, sizeof(pinkDenom)/sizeof(double), pinkDenom);
	//this is a butterworth filter designed with the following numpy code:
	//b, a = butter(1, 0.002)
	//this is not 100% accurate, but it is doubtful that the error is audible.
	double brownNumer[] = {0.00313176, 0.00313176};
	double brownDenom[] = {1.0, -0.99373647};
	brownifier.configure(sizeof(brownNumer)/sizeof(double), brownNumer, sizeof(brownDenom)/sizeof(double), brownDenom);
	//the pinkifier and brownifier are too quiet, so we bump them up some.
	//these numbers were obtained by listening to the output and adjusting.
	//if users need to have the noise on the range-1.0 to 1.0, they can use should_normalize=Truewhich works for all but absurdly small block sizes.
	pinkifier.setGain(4.0);
	brownifier.setGain(12.0);
	appendOutputConnection(0, 1);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createNoiseNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<NoiseNode>(simulation);
}

void NoiseNode::white() {
	for(int i = 0; i < block_size; i++) {
		//if this proves too costly, revisit and examine the box-muller method.
		float noiseSample = 0.0;
		do {
			noiseSample = normal_distribution(random_number_generator);
		} while(noiseSample < -1.0f || noiseSample > 1.0f);
		output_buffers[0][i] = noiseSample;
	}
}

void NoiseNode::pink() {
	white();
	for(int i = 0; i < block_size; i++) output_buffers[0][i] =pinkifier.tick(output_buffers[0][i]);
	//pass over the output buffer and find the max sample.
	for(int i = 0; i < block_size; i++) {
		if(fabs(output_buffers[0][i]) > pink_max) pink_max= fabs(output_buffers[0][i]);
	}
	if(getProperty(Lav_NOISE_SHOULD_NORMALIZE).getIntValue() == 0 || pink_max == 0.0f) return;
	for(int i = 0; i < block_size; i++) output_buffers[0][i]/=pink_max;
}

void NoiseNode::brown() {
	white();
	for(int i= 0; i < block_size; i++) output_buffers[0][i]=brownifier.tick(output_buffers[0][i]);
	//do something to make brown noise here...
	//pass over the output buffer and find the max sample.
	for(int i = 0; i < block_size; i++) {
		if(fabs(output_buffers[0][i]) > brown_max) brown_max= fabs(output_buffers[0][i]);
	}
	if(getProperty(Lav_NOISE_SHOULD_NORMALIZE).getIntValue() == 0 || brown_max == 0.0f) return;
	for(int i = 0; i < block_size; i++) output_buffers[0][i]/=brown_max;
}

void NoiseNode::process() {
	int type = getProperty(Lav_NOISE_NOISE_TYPE).getIntValue();
	switch(type) {
		case Lav_NOISE_TYPE_WHITE: white(); break;
		case Lav_NOISE_TYPE_PINK: pink(); break;
		case Lav_NOISE_TYPE_BROWN: brown(); break;
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createNoiseNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createNoiseNode(simulation);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}