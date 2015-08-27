/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <math.h>
#include "../private/constants.hpp"

namespace libaudioverse_implementation {

class OnePoleFilter {
	public:
	OnePoleFilter(double _sr): sr(_sr) {}
	float tick(float input);
	//Set the pole's position on the  real axis.
	void setPolePosition(float pos, bool normalize = true);
	//Set the filter up. Specify highpass or lowpass and the -3 db frequency.
	void setPoleFromFrequency(float fc, bool isHighpass = false);
	void setCoefficients(float b0, float a1);
	void reset();

	OnePoleFilter* getSlave();
	void setSlave(OnePoleFilter* s);
	float b0 = 1.0, a1 = 0.0;
	private:
	double sr = 0.0;
	//the history.
	float last = 0.0;
	OnePoleFilter* slave = nullptr;
};

inline float OnePoleFilter::tick(float input) {
	float out = b0*input-a1*last;
	last = out;
	return out;
}

inline void OnePoleFilter::setPolePosition(float pos, bool normalize) {
	float b0, a1;
	a1 = -pos;
	b0 = 1.0f;
	/**Reasoning time:
	The maximum gain of the filter is at dc or nyquist, introducing either a term of -1 or a term of 1.
	The maximum gain of the filter occurs when the denominator is minimized. The denominator is minimized when a1 is negative. So:
	max = 1/(1-|a1|)
	We wish to normalize the filter with b0. This means we wish to set b0 = 1/max.
	By the rules of fractions:
	b0 = 1-|a1|.
	*/
	if(normalize) b0 = (float)(1.0-abs(a1));
	setCoefficients(b0, a1);
}

inline void OnePoleFilter::setPoleFromFrequency(float fc, bool isHighpass) {
	float bandwidth;
	if(isHighpass) bandwidth = sr/2.0-fc;
	else bandwidth = fc;
	//Get the pole radius from the bandwidth.  We deal with negations in a moment.
	float rad = (float)exp(-PI*bandwidth/sr);
	//If this is a lowpass, the pole is in the right half of the z-plane. Otherwise it's in the left half.
	if(isHighpass) rad = -rad;
	setPolePosition(rad);
}

inline void OnePoleFilter::setCoefficients(float b0, float a1) {
	this->b0 = b0;
	this->a1 = a1;
	if(slave) slave->setCoefficients(b0, a1);
}

inline void OnePoleFilter::reset() {
	last = 0.0;
	if(slave) slave->reset();
}

inline OnePoleFilter* OnePoleFilter::getSlave() {
	return slave;
}

inline void OnePoleFilter::setSlave(OnePoleFilter* s) {
	slave = s;
}

}