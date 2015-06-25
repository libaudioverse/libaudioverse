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
	lines = new CrossfadingDelayLine*[n];
	for(int i = 0; i < n; i++) lines[i] = new CrossfadingDelayLine(maxDelay, sr);
	matrix = new float[n*n]();
	workspace = new float[n*n]();
}

FeedbackDelayNetwork::~FeedbackDelayNetwork() {
	for(int i = 0; i < n; i++) delete lines[i];
	delete[] lines;
	delete[] matrix;
	delete[] workspace;
}

void FeedbackDelayNetwork::computeFrame(float* outputs) {
	for(int i = 0; i < n; i++) {
		outputs[i] = lines[i]->computeSample();
	}
}

void FeedbackDelayNetwork::advance(const float* inputs, const float* lastOutputFrame) {
	for(int i=0; i < n; i++) {
		float sample=0.0f;
		for(int j = 0; j < n; j++) sample += matrix[i*n+j]*lastOutputFrame[j];
		sample+=inputs[i];
		lines[i]->advance(sample);
	}
}

//below here is not very algorithmic, just setters.

void FeedbackDelayNetwork::setMatrix(const float* feedbacks) {
	std::copy(feedbacks, feedbacks+n*n, matrix);
}

void FeedbackDelayNetwork::setDelays(const float* delays) {
	for(int i = 0; i < n; i++) setDelay(i, delays[i]);
}

void FeedbackDelayNetwork::setDelay(int which, float newDelay) {
	lines[which]->setDelay(newDelay);
}

void FeedbackDelayNetwork::setDelayCrossfadingTime(float time) {
	for(int i = 0; i < n; i++) lines[i]->setInterpolationTime(time);
}

void FeedbackDelayNetwork::reset() {
	for(int i = 0; i < n; i++) lines[i]->reset();
}

}