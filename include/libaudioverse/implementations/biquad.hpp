/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <kiss_fftr.h>

namespace libaudioverse_implementation {

/**Header-only biquad, and design function.
This class needs to be as fast as possible, and we want inlining whenever possible.
*/

class BiquadFilter {
	public:
	BiquadFilter(float _sr): sr(_sr) {}
	float tick(float input);

	void configure(int type, double frequency, double dbGain, double q);
	void clearHistories();
	double qFromBw(double frequency, double bw);
	double qFromS(double dbgain, double s);
	
	double b0 = 1.0, b1 = 0.0, b2 = 0.0;
	double a1 = 0.0, a2 = 0.0;
	private:
	float sr;
	//The history.
	double h1=0.0f, h2=0.0f;
};

//separate so it can be used by both this and the IIR filter.
void biquadConfigurationImplementation(double sr, int type, double frequency, double dbGain, double q, double &b0, double &b1, double &b2, double &a0, double &a1, double &a2);

inline float BiquadFilter::tick(float input)  {
	//Direct form 2: apply the  recursive  filter first.
	float recursive = input-a1*h1-a2*h2;
	//Apply the numerator, a simple convolution:
	float output = b0*recursive+b1*h1+b2*h2;
	h2=h1;
	h1=recursive;
	return output;
}

}