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
#include <libaudioverse/private/iir.hpp>
#include <limits>
#include <random>

class LavNoiseNode: public LavNode {
	public:
	LavNoiseNode(std::shared_ptr<LavSimulation> simulation);
	virtual void process();
	void white();
	void pink();
	void brown();
	std::mt19937_64 random_number_generator;
	std::uniform_real_distribution<float> uniform_distribution;
	LavIIRFilter pinkifier; //filter to turn white noise into pink noise.
	LavIIRFilter brownifier; //and likewise for brown.
	float pink_max = 0.0f, brown_max = 0.0f; //used for normalizing noise.
};

//we give the random number generator a fixed seed for debugging purposes.
LavNoiseNode::LavNoiseNode(std::shared_ptr<LavSimulation> simulation): LavNode(Lav_NODETYPE_NOISE, simulation, 0, 1),
random_number_generator(1234), uniform_distribution(-1.0f, 1.0f)  {
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
}

std::shared_ptr<LavNode> createNoiseNode(std::shared_ptr<LavSimulation> simulation) {
	std::shared_ptr<LavNoiseNode> retval = std::shared_ptr<LavNoiseNode>(new LavNoiseNode(simulation), LavObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void LavNoiseNode::white() {
	for(int i = 0; i < block_size; i++) {
		float noiseSample = 0.0f;
		//use the central limit theorem to generate white noise.
		//if this proves too costly, revisit and examine the box-muller method.
		for(int j = 0; j < 20; j++) noiseSample += uniform_distribution(random_number_generator);
		//divide by 20 to get it back into range.
		noiseSample /= 20.0f;
		output_buffers[0][i] = noiseSample;
	}
}

void LavNoiseNode::pink() {
	white();
	for(int i = 0; i < block_size; i++) output_buffers[0][i] =pinkifier.tick(output_buffers[0][i]);
	//pass over the output buffer and find the max sample.
	for(int i = 0; i < block_size; i++) {
		if(fabs(output_buffers[0][i]) > pink_max) pink_max= fabs(output_buffers[0][i]);
	}
	if(getProperty(Lav_NOISE_SHOULD_NORMALIZE).getIntValue() == 0 || pink_max == 0.0f) return;
	for(int i = 0; i < block_size; i++) output_buffers[0][i]/=pink_max;
}

void LavNoiseNode::brown() {
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

void LavNoiseNode::process() {
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
	auto simulation =incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createNoiseNode(simulation);
	*destination = outgoingObject<LavNode>(retval);
	PUB_END
}
