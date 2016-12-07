/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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