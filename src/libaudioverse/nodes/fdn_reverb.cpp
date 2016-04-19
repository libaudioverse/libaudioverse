/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/fdn_reverb.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/implementations/one_pole_filter.hpp>
#include <libaudioverse/implementations/interpolated_random_generator.hpp>
#include <algorithm>
#include <random> //we need the random sequence.

namespace libaudioverse_implementation {

/**This is a reverb based off a householder reflectiona bout the vector [1, 1, 1, 1, 1...].
We implement the reflection directly in order to avoid needing a full FDN.
*/

//constant data
const float delays[8] = {
	1933.0/44100.0,
	2143.0/44100.0,
	2473.0/44100.0,
	2767.0/44100.0,
	3217.0/44100.0,
	3557.0/44100.0,
	3907/44100.0,
	4127.0/44100.0,
};

//Controls density to delay multiplier conversion as:
//min_delay_multiplier+delay_multiplier_variation(1-density).
const float min_delay_multiplier = 0.3, delay_multiplier_variation = 1.4;
//Controls the maximum modulation.
//Must be less than the smallest possible delay line, or 0.3*delays[0].
//This value was determined through experimentation, such that the modulationn depth property maps to something reasonable at 1 with the default modulation frequency.
float modulation_duration = 0.01f;

FdnReverbNode::FdnReverbNode(std::shared_ptr<Simulation> sim): Node(Lav_OBJTYPE_FDN_REVERB_NODE, sim, 4, 4) {
	std::fill(feedback_gains, feedback_gains+8, 0.0f);
	delay_lines = new InterpolatedDelayLine*[8];
	delay_line_modulators = new InterpolatedRandomGenerator*[8];
	lowpass_filters = new OnePoleFilter*[8]();
	double sr = simulation->getSr();
	int seeds[8];
	std::seed_seq seq{1, 2, 3, 4, 5, 6, 7, 8};
	seq.generate(seeds, seeds+8);
	for(int  i = 0; i < 8; i++) {
		delay_lines[i] = new InterpolatedDelayLine(1.0, sr);
		delay_line_modulators[i] = new InterpolatedRandomGenerator(sr, seeds[i]);
		lowpass_filters[i] = new OnePoleFilter(sr);
	}
	appendInputConnection(0, 4);
	appendOutputConnection(0, 4);
	getProperty(Lav_FDN_REVERB_CUTOFF_FREQUENCY).setFloatRange(0.0f, sr/2.0);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createFdnReverbNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<FdnReverbNode>(simulation);
}

FdnReverbNode::~FdnReverbNode() {
	for(int i = 0; i < 8; i++) {
		delete delay_lines[i];
		delete lowpass_filters[i];
	}
	delete[] delay_lines;
	delete[] lowpass_filters;
}

void FdnReverbNode::modulateLines() {
	if(needs_modulation == false) return;
	for(int i = 0; i < 8; i++) {
		delay_lines[i]->setDelay(current_delays[i]+modulation_depth*delay_line_modulators[i]->tick());
	}
}

void FdnReverbNode::process() {
	if(werePropertiesModified(this, Lav_FDN_REVERB_T60, Lav_FDN_REVERB_CUTOFF_FREQUENCY,
	Lav_FDN_REVERB_DENSITY,
	Lav_FDN_REVERB_DELAY_MODULATION_FREQUENCY, Lav_FDN_REVERB_DELAY_MODULATION_DEPTH
	)) reconfigureModel();
	float lineValues[8];
	float feedbacks[8];
	for(int sample = 0; sample < block_size; sample++) {
		/*Since the vector is [1, 1, 1, 1, 1...] we have already multiplied by it.
		See https://ccrma.stanford.edu/~jos/pasp/Householder_Feedback_Matrix.html for the formulas we're using heere.
		In the form we use, a householder matrix has a diagonal of (1-2/n) and a nondiagonal of -2/n.
		In this algorithm, n is 8.
		*/
		//First, read from the lines.
		for(int i = 0; i < 8; i++) lineValues[i] = delay_lines[i]->computeSample();
		//The line values are output at this point.
		for(int i = 0; i < 8; i++) output_buffers[i%4][sample] = lineValues[i];
		//Do the feedback path.
		//Sum them to get the value of the row as though it were positive and 2/n all the way across.
		float lineSum = 0.0f;
		for(int i = 0; i < 8; i++) lineSum += lineValues[i];
		lineSum *= 0.25;
		/**The first row of a householder matrix is:
		(1-e), -e, -e, -e, -e, -e, -e, -e
		Where e=2/n.
		Subsequent rows simply move the (1-e) term.
		We have the result of the -e vector, essentially, and need only account for the missing 1. Consequently, the following.
		*/
		for(int i = 0; i < 8; i++) {
			feedbacks[i] = lineValues[i]-lineSum;
			feedbacks[i] *= feedback_gains[i];
			feedbacks[i] = lowpass_filters[i]->tick(feedbacks[i]);
		}
		//Bring the inputs in to the first four lines.
		for(int i = 0; i < 8; i++) feedbacks[i] += input_buffers[i%4][sample];
		//Write outputs.
		for(int i = 0; i < 8; i++) delay_lines[i]->advance(feedbacks[i]);
		modulateLines();
	}
}

void FdnReverbNode::reconfigureModel() {
	float t60 = getProperty(Lav_FDN_REVERB_T60).getFloatValue();
	float cutoff = getProperty(Lav_FDN_REVERB_CUTOFF_FREQUENCY).getFloatValue();
	float density = getProperty(Lav_FDN_REVERB_DENSITY).getFloatValue();
	float dbPerSec = -60.0f/t60;
	float delayMultiplier = min_delay_multiplier+delay_multiplier_variation*(1.0-density);
	for(int i = 0; i < 8; i++) {
		current_delays[i] = delayMultiplier*delays[i];
		float dbPerDelay = current_delays[i]*dbPerSec;
		float gain = dbToScalar(dbPerDelay, 1.0);
		feedback_gains[i] = gain;
		delay_lines[i]->setDelay(current_delays[i]);
		lowpass_filters[i]->setPoleFromFrequency(cutoff);
	}
	//Do we need to modulate.
	float modDepth = getProperty(Lav_FDN_REVERB_DELAY_MODULATION_DEPTH).getFloatValue();
	float modFreq = getProperty(Lav_FDN_REVERB_DELAY_MODULATION_FREQUENCY).getFloatValue();
	if(modDepth == 0.0 || modFreq == 0.0) {
		needs_modulation = false;
	}
	else {
		needs_modulation = true;
		modulation_depth = modDepth*modulation_duration;
		for(int i = 0; i < 8; i++)  delay_line_modulators[i]->setFrequency(modFreq);
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createFdnReverbNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createFdnReverbNode(simulation);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

}