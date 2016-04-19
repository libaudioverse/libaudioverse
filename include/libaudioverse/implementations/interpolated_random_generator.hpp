/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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