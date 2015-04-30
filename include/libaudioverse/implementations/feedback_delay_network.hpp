/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

namespace libaudioverse_implementation {

/**A feedback delay network consists of the following:
N delay lines of some quality or type.
A vector of length n representing relative output gains.
An n*n matrix of feedbacks.
An n*n matrix of feedback delays.

This last is nonstandard, but one common use case for this system is representing reverb and waveguide networks.
In such systems, it is needed to simulate sound waves travelling outward from a point, which can be thought of as two things:
- A feedback value representing attenuation.
- A feedback writing position representing the time it took to arrive.
Until the first time this matrix is set, it is not used. Once code sets this matrix, it is the responsibility of that code to manage it, not us.

Unlike normal delay lines, the advance funciton requires both an input frame and the last output frame.
This allows the last output frame to be modified before feeding it back in.
*/


class FeedbackDelayNetwork {
	public:
	FeedbackDelayNetwork(int n, float maxDelay, float sr);
	~FeedbackDelayNetwork();
	void computeFrame(float* outputs);
	void advance(float* nextInput, float* lastOutput);
	void setFeedbackMatrix(float* feedbacks);
	void setFeedbackDelayMatrix(float* feedbackDelays);
	void setDelays(float* delays);
	void setCrossfadingTime(float time);
	void reset();
	private:
	//fills first row of workspace with a frame of feedbacks for bank.
	void computeFeedbacks(float* lastInput);
	void advanceNoFeedbackDelayMatrix(float* lastInput, float* nextOutput);
	void advanceFeedbackDelayMatrix(float* nextInput, float* lastOutput);
	int n;
	float sr;
	CrossfadingDelayLine **bank = nullptr;
	float *feedback_matrix = nullptr, *feedback_delay_matrix = nullptr;
	//avoids very continuous reallocation, an n*n matrix.
	float* workspace = nullptr;
	float* delays = nullptr;
	bool using_feedback_delay_matrix = false;
};
}