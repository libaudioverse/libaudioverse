/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include <math.h>
#include <algorithm>
#include <stdio.h>
#include "../private/constants.hpp"

namespace libaudioverse_implementation {

class FirstOrderFilter {
	public:
	FirstOrderFilter(double _sr): sr(_sr) {}
	float tick(float input);
	//Set the pole's position on the  real axis.
	void setPolePosition(float pos, bool shouldNormalize = true);
	float getPolePosition();
	//Set the zero's position on the real axis.
	void setZeroPosition(float pos, bool shouldNormalize = true);
	float getZeroPosition();
	//Make the peak gain of this filter 1.
	//_useSlave is for internal use only.
	void normalize(bool _useSlave = false);
	//Configure as butterworth lowpass and highpass. Specify the 3 DB frequency.
	void configureLowpass(float frequency);
	void configureHighpass(float frequency);
	//Configure as a first-order allpass.  Phase of pi at DC, 0 at Nyquist, specify the pi/2 frequency.
	void configureAllpass(float frequency);
	void setCoefficients(float b0, float b1, float a1);
	void reset();	
	FirstOrderFilter* getSlave();
	void setSlave(FirstOrderFilter* s);
	
	float b0 = 1.0, b1 = 0.0, a1 = 0.0;
	private:
	double sr = 0.0;
	//the history.
	float lastOutput = 0.0, lastInput = 0.0;
	//All calls with side effects save tick forward to the slave, if set.
	FirstOrderFilter* slave = nullptr;
};

inline float FirstOrderFilter::tick(float input) {
	float out = b0*input+b1*lastInput-a1*lastOutput;
	lastInput = input;
	lastOutput = out;
	return out;
}

inline void FirstOrderFilter::setPolePosition(float pos, bool shouldNormalize) {
	a1 = -pos;
	if(shouldNormalize) normalize(false);
	if(slave) slave->setPolePosition(pos, shouldNormalize);
}

float FirstOrderFilter::getPolePosition() {
	return -a1;
}

inline void FirstOrderFilter::setZeroPosition(float pos, bool shouldNormalize) {
	b0 = 1.0f;
	b1 = -pos;
	if(shouldNormalize) normalize(false);
	if(slave) slave->setZeroPosition(pos, shouldNormalize);
}

inline float FirstOrderFilter::getZeroPosition() {
	return -b1/b0;
}

inline void FirstOrderFilter::normalize(bool _useSlave) {
	/**This filter can only have real poles and zeros.
	Consequently, peak gain is either at 0 or nyquist.
	We evaluate it as |(b_0+b_1g)/(1+a_1g)|
	where g is either 1 or -1.*/
	double g1 = abs((b0+b1)/(1+a1));
	double g2 = abs((b0-b1)/(1-a1));
	float g = std::max(g1, g2);
	//The gain is the max gain of the filter.
	//To normalize, multiply the input by 1/g.
	//We do so by folding into the numerator.
	//This can't affect bandpass filters, as bandpass has peak gain of 1. We do it by folding into b0.
	b0 /= g;
	b1 /= g;
	if(slave && _useSlave) slave->normalize();
}

inline void FirstOrderFilter::configureLowpass(float frequency) {
	float b0, b1, a1;
	b1 = 0.0;
	a1 = -exp(-PI*frequency/sr);
	b0 = 1.0+a1;
	setCoefficients(b0, b1, a1);
}

inline void FirstOrderFilter::configureHighpass(float frequency) {
	float b0, b1, a1;
	b1 = 0.0;
	float bandwidth = sr/2.0-frequency;
	a1 = exp(-PI*bandwidth/sr);
	b0 = 1-a1;
	setCoefficients(b0, b1, a1);
}

inline void FirstOrderFilter::configureAllpass(float frequency) {
	float b0, b1, a1;
	float omegaB = frequency*2*PI; //Not normalized because we're doing the bilinear transform.
	float omegaBT = omegaB/sr; //omegaB times the sampling interval.
	//Formulas from Physical Audio Processing by JOS. See section on analog phasing: https://ccrma.stanford.edu/~jos/pasp/Virtual_Analog_Example_Phasing.html
	//The above link derives the positionn of the pole and 0 from the bilinear transform, giving the following formulas.
	float pole = (1.0-tan(omegaBT/2.0))/(1+tan(omegaBT/2.0));
	b0 = pole;
	b1 = 1.0;
	a1 = pole;
	setCoefficients(b0, b1, a1);
}

inline void FirstOrderFilter::setCoefficients(float b0, float b1, float a1) {
	this->b0 = b0;
	this->b1 = b1;
	this->a1 = a1;
	if(slave) slave->setCoefficients(b0, b1, a1);
}

inline void FirstOrderFilter::reset() {
	lastOutput = 0.0;
	lastInput = 0.0;
	if(slave) slave->reset();
}

inline FirstOrderFilter* FirstOrderFilter::getSlave() {
	return slave;
}

inline void FirstOrderFilter::setSlave(FirstOrderFilter* s) {
	slave = s;
}

}