/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/implementations/feedback_delay_network.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

FeedbackDelayNetwork::FeedbackDelayNetwork(int n, float maxDelay, float sr) {
	this->n = n;
	this->sr = sr;
	this->using_feedback_delay_matrix = false;
	bank = new CrossfadingDelayLine*[n];
	for(int i = 0; i < n; i++) bank[i] = new CrossfadingDelayLine(maxDelay, sr);
	feedback_matrix = new float[n*n]();
	feedback_delay_matrix = new float[n*n]();
	delays = new float[n]();
	workspace = new float[n*n]();
}

FeedbackDelayNetwork::~FeedbackDelayNetwork() {
	for(int i = 0; i < n; i++) delete bank[i];
	delete[] bank;
	delete[] feedback_matrix;
	delete[] feedback_delay_matrix;
	delete[] delays;
	delete[] workspace;
}

void FeedbackDelayNetwork::computeFrame(float* outputs) {
	//the output step is not special.
	for(int i = 0; i < n; i++) outputs[i] = bank[i]->computeSample();
}

void FeedbackDelayNetwork::advance(float* nextInput, float* lastOutput) {
	//two cases. The first is that we're not using the feedback delay matrix.
	if(using_feedback_delay_matrix == false) {
		advanceNoFeedbackDelayMatrix(nextInput, lastOutput);
	} else {
		advanceFeedbackDelayMatrix(nextInput, lastOutput);
	}
}

void FeedbackDelayNetwork::computeFeedbacks(float* lastOutput) {
	for(int row = 0; row < n; row++) {
		for(int column = 0; column < n; column++) {
			workspace[row*n+column] = lastOutput[row]*feedback_matrix[row*n+column];
		}
	}
	//now we sum them in the first row.
	for(int column = 0; column< n; column++) {
		for(int row = 1; row < n; row++) {
			workspace[column] += workspace[row*n+column];
		}
	}
	//and now the first row of workspace is a frame of feedback.
}

void FeedbackDelayNetwork::advanceNoFeedbackDelayMatrix(float* nextInput, float* lastOutput) {
	//in this case, we advance the delay lines with their computed feedback samples.
	//fill workspace with a computed feedback frame:
	computeFeedbacks(lastOutput);
	//sum onto workspace the next input frame.
	for(int column = 0; column < n; column++) workspace[column]+=nextInput[column];
	//and feed this to the bank.
	for(int column = 0; column < n; column++) bank[column]->advance(workspace[column]);
}

void FeedbackDelayNetwork::advanceFeedbackDelayMatrix(float* nextInput, float* lastOutput) {
	//in this case, we do the same as above.
	//but instead of advancing with feedback+input, we advance with input and then manually write the feedbacks.
	for(int column = 0; column < n; column++) bank[column]->advance(nextInput[column]);
	//fills workspace as before.
	computeFeedbacks(lastOutput);
	//now, we write.
	for(int column = 0; column < n; column++) {
		for(int row = 0; row < n; row++) {
			bank[column]->add(feedback_delay_matrix[column*n+row], workspace[column]);
		}
	}
}

//below here is not very algorithmic, just setters.

void FeedbackDelayNetwork::setFeedbackMatrix(float* feedbacks) {
	std::copy(feedbacks, feedbacks+n*n, feedback_matrix);
}

void FeedbackDelayNetwork::setFeedbackDelayMatrix(float* feedbackDelays) {
	std::copy(feedbackDelays, feedbackDelays+n*n, feedback_delay_matrix);
}

void FeedbackDelayNetwork::setDelays(float* delays) {
	std::copy(delays, delays+n, this->delays);
	for(int i = 0; i < n; i++) bank[i]->setDelay(delays[i]);
}

void FeedbackDelayNetwork::setCrossfadingTime(float time) {
	for(int i = 0; i < n; i++) bank[i]->setInterpolationTime(time);
}

void FeedbackDelayNetwork::reset() {
	for(int i = 0; i < n; i++) bank[i]->reset();
}

}