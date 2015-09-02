/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/implementations/one_pole_filter.hpp>
#include <algorithm>

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

class FdnReverbNode: public Node {
	public:
	FdnReverbNode(std::shared_ptr<Simulation> sim);
	~FdnReverbNode();
	void process();
	void reconfigureModel();
	float feedback_gains[8];
	OnePoleFilter** lowpass_filters = nullptr;
	InterpolatedDelayLine** delay_lines = nullptr;
};

FdnReverbNode::FdnReverbNode(std::shared_ptr<Simulation> sim): Node(Lav_OBJTYPE_FDN_REVERB_NODE, sim, 4, 4) {
	std::fill(feedback_gains, feedback_gains+8, 0.0f);
	delay_lines = new InterpolatedDelayLine*[8];
	lowpass_filters = new OnePoleFilter*[8]();
	double sr = simulation->getSr();
	for(int  i = 0; i < 8; i++) {
		delay_lines[i] = new InterpolatedDelayLine(1.0, sr);
		lowpass_filters[i] = new OnePoleFilter(sr);
	}
	appendInputConnection(0, 4);
	appendOutputConnection(0, 4);
	getProperty(Lav_FDN_REVERB_CUTOFF_FREQUENCY).setFloatRange(0.0f, sr/2.0);
}

std::shared_ptr<Node> createFdnReverbNode(std::shared_ptr<Simulation> simulation) {
	auto retval = std::shared_ptr<FdnReverbNode>(new FdnReverbNode(simulation), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

FdnReverbNode::~FdnReverbNode() {
	for(int i = 0; i < 8; i++) {
		delete delay_lines[i];
		delete lowpass_filters[i];
	}
	delete[] delay_lines;
	delete[] lowpass_filters;
}

void FdnReverbNode::process() {
	if(werePropertiesModified(this, Lav_FDN_REVERB_T60, Lav_FDN_REVERB_CUTOFF_FREQUENCY)) reconfigureModel();
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
		//And finally we write.
		for(int i = 0; i < 8; i++) delay_lines[i]->advance(feedbacks[i]);
	}
}

void FdnReverbNode::reconfigureModel() {
	float t60 = getProperty(Lav_FDN_REVERB_T60).getFloatValue();
	float cutoff = getProperty(Lav_FDN_REVERB_CUTOFF_FREQUENCY).getFloatValue();
	float dbPerSec = -60.0f/t60;
	for(int i = 0; i < 8; i++) {
		float dbPerDelay = delays[i]*dbPerSec;
		float gain = dbToScalar(dbPerDelay, 1.0);
		feedback_gains[i] = gain;
		delay_lines[i]->setDelay(delays[i]);
		lowpass_filters[i]->setPoleFromFrequency(cutoff);
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