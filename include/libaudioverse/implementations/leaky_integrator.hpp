/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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