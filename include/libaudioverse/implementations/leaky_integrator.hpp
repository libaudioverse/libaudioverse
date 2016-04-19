/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include <cmath>

namespace libaudioverse_implementation {
/**A leaky integrator.

This integrator integrates the input signal, leaking a specified percent over a period of 1 second.
*/
class LeakyIntegrator {
	public:
	LeakyIntegrator(float _sr);
	float tick(float input);
	//Set the leakyness in percent per second.
	void setLeakyness(double percent);
	void reset();
	private:
	double integral = 0.0, dx = 0.0, leakFactor = 1.0;
	float sr = 0.0f;
};

inline LeakyIntegrator::LeakyIntegrator(float _sr): sr(_sr), dx(1.0/_sr) {}

inline float LeakyIntegrator::tick(float input) {
	double out = integral*leakFactor+dx*input;
	integral = out;
	return (float) out;
}

inline void LeakyIntegrator::setLeakyness(double percent) {
	//We have percent per second, we want percent per sample.  That is:
	//x^sr = percent, x=percent^(1/sr)
	leakFactor = std::pow(percent, 1.0/sr);
}

inline void LeakyIntegrator::reset() {
	integral = 0.0;
}
}