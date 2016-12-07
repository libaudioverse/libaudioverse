/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include <random>

namespace libaudioverse_implementation {

/**This generates normally distributed random numbers between -1 and 1 at a specified frequency.

Values between two adjacent random numbers use linear interpolation.
The default frequency is 1 HZ.
The standard deviation is 0.5, and values greater than 1 in absolute value are skipped.*/
class InterpolatedRandomGenerator {
	public:
	InterpolatedRandomGenerator(float sr, int seed);
	void setFrequency(float freq);
	float tick();
	private:
	float computeRandomNumber();
	float n1 = 0.0f, n2 = 0.0f;
	float w1 = 1.0f, w2 = 0.0f;
	float delta = 0.0f;
	float sr;
	std::minstd_rand engine;
	std::normal_distribution<float> distribution;
};

InterpolatedRandomGenerator::InterpolatedRandomGenerator(float sr, int seed):
engine(seed),
distribution(0.0, 0.5) {
	this->sr = sr;
	setFrequency(1);
	n2 = computeRandomNumber();
}

float InterpolatedRandomGenerator::tick() {
	float ret = w1*n1+w2*n2;
	w1 -= delta;
	w2 += delta;
	if(w2 > 1.0f) {
		n1 = n2;
		n2 = computeRandomNumber();
		w1 = 1.0f;
		w2 = 0.0f;
	}
	return ret;
}

void InterpolatedRandomGenerator::setFrequency(float freq) {
	delta = freq/sr;
}

float InterpolatedRandomGenerator::computeRandomNumber() {
	float rnd = 0.0f;
	do {
		rnd = distribution(engine);
	} while (rnd < -1.0f || rnd > 1.0f);
	return rnd;
}

}