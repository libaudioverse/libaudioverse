/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once

namespace libaudioverse_implementation {

/**Header-only biquad, and design function.
This class needs to be as fast as possible, and we want inlining whenever possible.
*/

class BiquadFilter {
	public:
	BiquadFilter(float _sr): sr(_sr) {}
	float tick(float input);

	void configure(int type, double frequency, double dbGain, double q);
	void reset();
	void setCoefficients(double b0, double b1, double b2, double a1, double a2);
	
	double qFromBw(double frequency, double bw);
	double qFromS(double dbgain, double s);

	BiquadFilter* getSlave();
	void setSlave(BiquadFilter* s);	
	
	double b0 = 1.0, b1 = 0.0, b2 = 0.0;
	double a1 = 0.0, a2 = 0.0;
	private:
	float sr;
	//The history.
	double h1=0.0f, h2=0.0f;
	//If set, all functions but tick with side effects forward onto the slave.
	BiquadFilter* slave = nullptr;
};

//separate so it can be used by both this and the IIR filter.
void biquadConfigurationImplementation(double sr, int type, double frequency, double dbGain, double q, double &b0, double &b1, double &b2, double &a0, double &a1, double &a2);

inline float BiquadFilter::tick(float input)  {
	//Direct form 2: apply the  recursive  filter first.
	double recursive = input-a1*h1-a2*h2;
	//Apply the numerator, a simple convolution:
	float output = (float)(b0*recursive+b1*h1+b2*h2);
	h2=h1;
	h1=recursive;
	return output;
}

}