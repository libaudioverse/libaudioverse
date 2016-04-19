/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "delayline.hpp"
#include "../private/kernels.hpp"

namespace libaudioverse_implementation {
/**A feedback delay network consists of the following:
N delay lines of some quality or type.
A vector of length n representing relative output gains.
An n*n matrix of feedbacks.

To advance this object, call computeFrame, add your inputs to the computed values, then call advance.
Many nodes will wish to customize this process via the insertion of filters, so it is left to those nodes to carry out this step.

This matches the formulation given in Physical Audio Processing for Virtual Musical Instruments and Audio Effects by julius O. Smith III.

We use a template here because it is necessary to change the type of the delay line.
Since the advance/read functions would be called n times per sample, virtual functions are unacceptible.
This has the side effect of moving everything into the header.
*/
template <class LineType=CrossfadingDelayLine>
class FeedbackDelayNetwork {
	public:
	FeedbackDelayNetwork(int n, float maxDelay, float sr) {
		this->n = n;
		this->sr = sr;
		lines = new LineType*[n];
		for(int i = 0; i < n; i++) lines[i] = new LineType(maxDelay, sr);
		matrix = allocArray<float>(n*n);
	}
	
	~FeedbackDelayNetwork() {
	for(int i = 0; i < n; i++) delete lines[i];
		delete[] lines;
		freeArray(matrix);
	}
	
	void computeFrame(float* outputs) {
		for(int i = 0; i < n; i++) {
			outputs[i] = lines[i]->computeSample();
		}
	}
	
	void advance(const float* inputs, const float* lastOutputFrame) {
		for(int i=0; i < n; i++) {
			float sample=dotKernel(n, matrix+n*i, lastOutputFrame);
			sample+=inputs[i];
			lines[i]->advance(sample);
		}
	}
	
	void setMatrix(const float* feedbacks) {
		std::copy(feedbacks, feedbacks+n*n, matrix);
	}
	
	void setDelays(const float* delays) {
		for(int i = 0; i < n; i++) setDelay(i, delays[i]);
	}
	
	void setDelay(int which, float newDelay) {
		lines[which]->setDelay(newDelay);
	}
	
	LineType& getDelayLine(int which) {
		return *lines[which];
	}

	void reset() {
		for(int i = 0; i < n; i++) lines[i]->reset();
	}
	
	private:
	int n;
	float sr;
	LineType **lines = nullptr;
	float *matrix = nullptr;
};

}