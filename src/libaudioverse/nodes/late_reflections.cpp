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
#include <libaudioverse/private/iir.hpp>
#include <limits>
#include <algorithm>
#include <random>

namespace libaudioverse_implementation {

/**Algorithm explanation:
This algorithm consists of a FDN and two highshelf filters inserted in the feedback:
fdn->mid_highshelf->high_highshelf->fdn

We compute individual gains for each line, using math taken from Physical Audio Processing by JOS.

These gains are the "low band", and two shelving filters are then applied to shape the remaining two bands.

Unfortunately, the biquad formulas for lowshelf are unstable at low frequencies, something which needs debugging.
Consequently, we have to start from the lowest band and move up with highshelves.

The delay lines have coprime lengths.

In order to increase the accuracy of panning, only 16 unique delay line lengths are used.
Each delay is copied to fill a range, namely order/16, of adjacent lines.
*/

//The order must be a multiple of 16 and power of two.
const int order=32;

//Used for computing the delay line lengths.
//A set of coprime integers.
int coprimes[] = {
	3, 4, 5, 7,
	9, 11, 13, 16,
	17, 19, 23, 27,
	29, 31, 35, 37
};

class LateReflectionsNode: public Node {
	public:
	LateReflectionsNode(std::shared_ptr<Simulation> simulation);
	~LateReflectionsNode();
	virtual void process();
	void recompute();
	void reset() override;
	FeedbackDelayNetwork fdn;
	float* delays = nullptr;
	float *gains;
	float* output_frame=nullptr, *next_input_frame =nullptr;
	float* normalized_hadamard = nullptr;
	//Filters for the band separation.
	IIRFilter** highshelves; //Shapes from mid to high band.
	IIRFilter** midshelves; //Shapes from low to mid band.
};

LateReflectionsNode::LateReflectionsNode(std::shared_ptr<Simulation> simulation):
Node(Lav_OBJTYPE_LATE_REFLECTIONS_NODE, simulation, order, order),
fdn(order, 1.0f, simulation->getSr()) {
	for(int i=0; i < order; i++) {
		appendInputConnection(i, 1);
		appendOutputConnection(i, 1);
	}
	normalized_hadamard=allocArray<float>(order*order);
	//get a hadamard.
hadamard(order, normalized_hadamard);
	//feed the fdn the initial matrix.
	fdn.setMatrix(normalized_hadamard);
	//this is fixed...for now.
	fdn.setDelayCrossfadingTime(0.05);
	gains=allocArray<float>(order);
	output_frame=allocArray<float>(order);
	next_input_frame=allocArray<float>(order);
	delays=allocArray<float>(order);
	//range for hf and lf.
	double nyquist=simulation->getSr()/2.0;
	getProperty(Lav_LATE_REFLECTIONS_HF_REFERENCE).setFloatRange(0.0, nyquist);
	getProperty(Lav_LATE_REFLECTIONS_LF_REFERENCE).setFloatRange(0.0, nyquist);
	//allocate the filters.
	highshelves=new IIRFilter*[order];
	midshelves = new IIRFilter*[order];
	for(int i = 0; i < order; i++) {
		highshelves[i] = new IIRFilter(simulation->getSr());
		midshelves[i] = new IIRFilter(simulation->getSr());
	}
	//initial configuration.
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
	freeArray(next_input_frame);
	freeArray(delays);
	for(int i=0; i < order; i++) {
		delete highshelves[i];
		delete midshelves[i];
	}
	delete[] highshelves;
	delete[] midshelves;
}

double t60ToGain(double t60, double lineLength) {
	double dbPerSec= -60.0/t60;
	//Db decrease for one circulation of the delay line.
	double dbPerPeriod = dbPerSec*lineLength;
	//convert to a gain.
	return pow(10, dbPerPeriod/20.0);
}

void LateReflectionsNode::recompute() {
	float density = getProperty(Lav_LATE_REFLECTIONS_DENSITY).getFloatValue();
	float t60=getProperty(Lav_LATE_REFLECTIONS_T60).getFloatValue();
	float t60_high =getProperty(Lav_LATE_REFLECTIONS_HF_T60).getFloatValue();
	float t60_low =getProperty(Lav_LATE_REFLECTIONS_LF_T60).getFloatValue();
	float hf_reference=getProperty(Lav_LATE_REFLECTIONS_HF_REFERENCE).getFloatValue();
	float lf_reference = getProperty(Lav_LATE_REFLECTIONS_LF_REFERENCE).getFloatValue();
	//The base delay is the amount we are delaying all delay lines by.
	float baseDelay = 0.003+(1.0f-density)*0.03;
	//Approximate delay line lengths using powers of primes.
	for(int i = 0; i < 16; i+=1) {
		int prime= coprimes[i];
		//use change of base.
		double powerApprox = log(baseDelay*simulation->getSr())/log(prime);
		int neededPower=round(powerApprox);
		double delayInSamples = pow(prime, neededPower);
		double delay=delayInSamples/simulation->getSr();
		delay = std::min(delay, 1.0);
		delays[i] = delay;
	}
	for(int i=1; i < order/16; i++) {
		std::copy(delays, delays+16, delays+i*16);
		//reverse every other range.
		if(i %2) std::reverse(delays+i*16, delays+i*16+16);
	}
	//These are interpolated delay lines.
	//If we have "opposite" pairs with exactly the same delay, we get points in the reverb that do not sound like all others.
	//To that end, move them by half a sample.
	for(int i =0; i < order/16; i++) {
		float delta=1.0/simulation->getSr()/2.0;
		if(i%2) delta*=-1;
		delays[i*16]+=delta;
	}
	fdn.setDelays(delays);
	//configure the gains.
	for(int i= 0; i < order; i++) {
		gains[i] = t60ToGain(t60_low, delays[i]);
	}
	//Configure the filters.
	for(int i = 0; i < order; i++) {
		//We get the mid and high t60 gains, and turn them into db.
		double highGain=t60ToGain(t60_high, delays[i]);
		double midGain=t60ToGain(t60, delays[i]);
		double midDb=scalarToDb(midGain, gains[i]);
		double highDb = scalarToDb(highGain, midGain);
		//Careful reading of the audio eq cookbook reveals that when s=1, q is always sqrt(2).
		//We add a very tiny bit to help against numerical error.
		highshelves[i]->configureBiquad(Lav_BIQUAD_TYPE_HIGHSHELF, hf_reference, highDb, 1/sqrt(2.0)+1e-4);
		midshelves[i]->configureBiquad(Lav_BIQUAD_TYPE_HIGHSHELF, lf_reference, midDb, 1.0/sqrt(2.0)+1e-4);
	}
}

void LateReflectionsNode::process() {
	if(werePropertiesModified(this,
	Lav_LATE_REFLECTIONS_T60, Lav_LATE_REFLECTIONS_DENSITY, Lav_LATE_REFLECTIONS_HF_T60,
	Lav_LATE_REFLECTIONS_LF_T60, Lav_LATE_REFLECTIONS_HF_REFERENCE, Lav_LATE_REFLECTIONS_LF_REFERENCE
	)) recompute();
	for(int i= 0; i < block_size; i++) {
		//Get the fdn's output.
		fdn.computeFrame(output_frame);
		for(int j= 0; j < order; j++) output_buffers[j][i] = output_frame[j];
		for(int j=0; j < order; j++)  {
			//Through the highshelf, then the lowshelf.
			output_frame[j] = midshelves[j]->tick(highshelves[j]->tick(gains[j]*output_frame[j]));
		}
		//bring in the inputs.
		for(int j = 0; j < order; j++) next_input_frame[j] = input_buffers[j][i];
		fdn.advance(next_input_frame, output_frame);
	}
}

void LateReflectionsNode::reset() {
	fdn.reset();
	for(int i = 0; i < order; i++) {
		midshelves[i]->clearHistories();
		highshelves[i]->clearHistories();
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