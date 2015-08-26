/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/biquad.hpp>
//Get the biquad types:
#include <libaudioverse/libaudioverse_properties.h>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

void BiquadFilter::configure(int type, double frequency, double dbGain, double q) {
	double a0, a1, a2, b0, b1, b2;
	//This function takes references.
	biquadConfigurationImplementation(sr, type, frequency, dbGain, q, b0, b1, b2, a0, a1, a2);
	//We want a0 to be 1.  We can do this by multiplying by 1/(1/a0).
	//Unfortunately, this changes the filter gain.  But suppose we multiply by (1/a0)/(1/a0) = 1:
	b0 /= a0;
	b1 /= a0;
	b2 /= a0;
	a1 /= a0;
	a2 /= a0;
	setCoefficients(b0, b1, b2, a1, a2);
}

void BiquadFilter::reset() {
	h1=0.0f;
	h2=0.0f;
	if(slave) slave->reset();
}

void BiquadFilter::setCoefficients(double b0, double b1, double b2, double a1, double a2) {
	this->b0 = b0;
	this->b1 = b1;
	this->b2 = b2;
	this->a1 = a1;
	this->a2 = a2;
	if(slave) slave->setCoefficients(b0, b1, b2, a1, a2);
}

double BiquadFilter::qFromBw(double frequency, double bw) {
	return frequency/bw;
}

double BiquadFilter::qFromS(double dbgain, double s) {
	double a=pow(10.0, dbgain/40.0);
	return sqrt((a+1/a)*(1/s-1) + 2);
}

BiquadFilter* BiquadFilter::getSlave() {
	return slave;
}

void BiquadFilter::setSlave(BiquadFilter* s) {
	slave = s;
}

void biquadConfigurationImplementation(double sr, int type, double frequency, double dbGain, double q, double &b0, double &b1, double &b2, double &a0, double &a1, double &a2) {
	//this entire function is a straightforward implementation of the Audio EQ cookbook, included with this repository.
	//alias our parameters to match the Audio EQ cookbook.
	double fs = sr;
	double f0 = frequency;
	//only common intermediate variable for all of these.
	double omega = 2.0*PI*f0/fs;
	double sinv=sin(omega);
	double cosv=cos(omega);
	double alpha = sinv/ (2.0 * q);
	double  a = sqrt(pow(10, dbGain/20.0)); //this is recalculated for 3 special cases later.
	double beta = 0.0; //recomputed below.
	switch(type) {
		case Lav_BIQUAD_TYPE_LOWPASS:
		b0 = (1 - cosv)/2.0;
		b1 = 1 - cosv;
		b2 = (1 -cosv) /2.0;
		a0 = 1 + alpha;
		a1 = -2.0 * cosv;
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_HIGHPASS:
		b0 = (1 + cosv)/2.0;
		b1 = -(1 + cosv);
		b2 = (1 + cosv) / 2.0;
		a0 = 1 + alpha;
		a1 = -2.0*cosv;
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_BANDPASS:
		b0 = alpha;
		b1 = 0;
		b2 = -alpha;
		a0 = 1 + alpha;
		a1 = -2.0*cosv;
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_NOTCH:
		b0 = 1;
		b1 = -2.0 * cosv;
		b2 = 1.0;
		a0 = 1 + alpha;
		a1 = -2.0 * cosv;
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_ALLPASS:
		b0 = 1 - alpha;
		b1 = -2.0 * cosv;
		b2 = 1 + alpha;
		a0 = 1 + alpha;
		a1 = -2.0 * cosv;
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_PEAKING:
		a = pow(10, dbGain/40.0);
		b0 = 1 + alpha*a;
		b1 = -2.0 * cosv;
		b2 = 1 - alpha * a;
		a0 = 1 + alpha / a;
		a1 = -2.0 * cosv;
		a2 = 1 - alpha / a;
		break;
		case Lav_BIQUAD_TYPE_LOWSHELF:
		a = pow(10, dbGain/40.0);
		beta = sqrt(a)/q;
		b0 = a*((a+1) - (a - 1)*cosv + beta*sinv);
		b1 = 2 * a * ((a - 1) - (a + 1) * cosv);
		b2 = a * ((a + 1) - (a - 1) * cosv - beta*sinv);
		a0 = (a + 1) + (a - 1)*cosv + beta*sinv;
		a1 = -2.0*((a - 1)+ (a+1) * cosv);
		a2 = (a + 1) - (a - 1)*cosv-beta*sinv;
		break;
		case Lav_BIQUAD_TYPE_HIGHSHELF:
		a = pow(10, dbGain/40.0);
		beta =sqrt(a)/q;
		b0 = a*((a+1)+(a-1)*cosv + beta*sinv);
		b1 = -2.0*a*((a - 1)+(a+1)*cosv);
		b2 = a*((a+1)+(a-1)*cosv-beta*sinv);
		a0 = (a+1) - (a - 1)*cosv+beta*sinv;
		a1 = 2.0*((a-1)-(a+1)*cosv);
		a2 = (a+1)-(a-1)*cosv-beta*sinv;
		break;
		case Lav_BIQUAD_TYPE_IDENTITY:
		b0 = 1;
		b1 = 0;
		b2 = 0;
		a0 = 1;
		a1 = 0;
		a2 = 0;
		break;
	};
}

}