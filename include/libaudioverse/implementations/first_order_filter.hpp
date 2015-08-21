/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <math.h>
#include "../private/constants.hpp"

namespace libaudioverse_implementation {

class FirstOrderFilter {
	public:
	FirstOrderFilter(double _sr): sr(_sr) {}
	float tick(float input);
	//Set the pole's position on the  real axis.
	void setPolePosition(float pos);
	float getPolePosition();
	//Set the zero's position on the real axis.
	void setZeroPosition(float pos);
	float getZeroPosition();
	//Configure as butterworth lowpass and highpass. Specify the 3 DB frequency.
	void configureLowpass(float frequency);
	void configureHighpass(float frequency);
	private:
	double sr = 0.0;
	float b0 = 1.0, b1 = 0.0, a1 = 0.0;
	//the history.
	float lastOutput = 0.0, lastInput = 0.0;
};

inline float FirstOrderFilter::tick(float input) {
	float out = b0*input+b1*lastInput-a1*lastOutput;
	lastInput = input;
	lastOutput = out;
	return out;
}

inline void FirstOrderFilter::setPolePosition(float pos) {
	a1 = -pos;
}

float FirstOrderFilter::getPolePosition() {
	return -a1;
}

inline void FirstOrderFilter::setZeroPosition(float pos) {
	b0 = 1.0f;
	b1 = -pos;
}

inline float FirstOrderFilter::getZeroPosition() {
	return -b1/b0;
}

inline void FirstOrderFilter::configureLowpass(float frequency) {
	b1 = 0.0;
	a1 = -exp(-2*PI*frequency/sr);
	b0 = 1.0+a1;
}

inline void FirstOrderFilter::configureHighpass(float frequency) {
	b1 = 0.0;
	a1 = exp(-2.0*PI*frequency/sr);
	b0 = 1-a1;
}

}