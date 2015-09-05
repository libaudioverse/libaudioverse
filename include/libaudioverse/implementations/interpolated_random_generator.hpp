/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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