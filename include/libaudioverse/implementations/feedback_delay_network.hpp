/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

namespace libaudioverse_implementation {
/**A feedback delay network consists of the following:
N delay lines of some quality or type.
A vector of length n representing relative output gains.
An n*n matrix of feedbacks.

To advance this object, call computeFrame, add your inputs to the computed values, then call advance.
Many nodes will wish to customize this process via the insertion of filters, so it is left to those nodes to carry out this step.

This matches the formulation given in Physical Audio Processing for Virtual Musical Instruments and Audio Effects by julius O. Smith III.
*/

class FeedbackDelayNetwork {
	public:
	FeedbackDelayNetwork(int n, float maxDelay, float sr);
	~FeedbackDelayNetwork();
	void computeFrame(float* outputs);
	void advance(float* inputs);
	void setMatrix(float* feedbacks);
	void setDelays(float* delays);
	void setDelayCrossfadingTime(float time);
	void reset();
	private:
	int n;
	float sr;
	CrossfadingDelayLine **lines = nullptr;
	float *matrix = nullptr;
	//avoids very continuous reallocation, an n*n matrix that can be used for any purpose.
	float* workspace = nullptr;
	float* delays = nullptr;
};

}