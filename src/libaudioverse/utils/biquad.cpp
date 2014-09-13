/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implement the biquad class.*/
#include <libaudioverse/private_biquad.hpp>
#include <libaudioverse/libaudioverse_properties.h>
#include <math.h>
#include <libaudioverse/private_constants.hpp>

void LavBiquad::configure(int type, double sr, double frequency, double dbGain, double q) {
	//this entire function is a straightforward implementation of the Audio EQ cookbook, included with this repository.
	//we move these onto the class at the end of the function explicitly.
	double a0, a1, a2, b0, b1, b2, gain;
	//alias our parameters to match the Audio EQ cookbook.
	double fs = sr;
	double f0 = frequency;
	//only common intermediate variable for all of these.
	double w0 = 2.0*PI*f0/fs;
	double alpha = sin(w0) / (2.0 * q);
	double  a = sqrt(pow(10, dbGain/20.0)); //this is recalculated for 3 special cases later.
	switch(type) {
		case Lav_BIQUAD_TYPE_LOWPASS:
		b0 = (1 - cos(w0))/2.0;
		b1 = 1 - cos(w0);
		b2 = (1 - cos(w0)) /2.0;
		a0 = 1 + alpha;
		a1 = -2.0 * cos(w0);
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_HIGHPASS:
		b0 = (1 + cos(w0))/2.0;
		b1 = -(1 + cos(w0));
		b2 = (1 + cos(w0)) / 2.0;
		a0 = 1 + alpha;
		a1 = -2.0*cos(w0);
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_BANDPASS:
		b0 = sin(w0) / 2.0;
		b1 = 0;
		b2 = -sin(w0)/2.0;
		a0 = 1 + alpha;
		a1 = -2.0*cos(w0);
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_NOTCH:
		b0 = 1;
		b1 = -2.0 * cos(w0);
		b2 = 1.0;
		a0 = 1 + alpha;
		a1 = -2.0 * cos(w0);
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_ALLPASS:
		b0 = 1 - alpha;
		b1 = -2.0 * cos(w0);
		b2 = 1 + alpha;
		a0 = 1 + alpha;
		a1 = -2.0 * cos(w0);
		a2 = 1 - alpha;
		break;
		case Lav_BIQUAD_TYPE_PEAKING:
		a = pow(10, dbGain/40.0);
		b0 = 1 + alpha*a;
		b1 = -2.0 * cos(w0);
		b2 = 1 - alpha * a;
		a0 = 1 + alpha / a;
		a1 = -2.0 * cos(w0);
		a2 = 1 - alpha / a;
		break;
		case Lav_BIQUAD_TYPE_LOWSHELF:
		a = pow(10, dbGain/40.0);
		b0 = a*((a+1) - (a - 1)*cos(w0) + 2.0 * sqrt(a) * alpha);
		b1 = 2 * a * ((a - 1) - (a + 1) * cos(w0));
		b2 = a * ((a + 1) - (a - 1) * cos(w0) - 2.0 * sqrt(a)*alpha);
		a0 = (a + 1) + (a - 1)*cos(w0) + 2*sqrt(a)*alpha;
		a1 = -2.0*((a - 1)+ (a+1) * cos(w0));
		a2 = (a + 1) - (a - 1)*cos(w0) - 2*sqrt(a)*alpha;
		break;
		case Lav_BIQUAD_TYPE_HIGHSHELF:
		a = pow(10, dbGain/40.0);
		b0 = a*((a+1)+(a-1)*cos(w0) + 2*sqrt(a)*alpha);
		b1 = -2.0*a*((a - 1)+(a+1)*cos(w0));
		b2 = a*((a+1)+(a-1)*cos(w0)-2*sqrt(a)*alpha);
		a0 = (a+1) - (a - 1)*cos(w0) + 2*sqrt(a)*alpha;
		a1 = 2.0*((a-1)-(a+1)*cos(w0));
		a2 = (a+1)-(a-1)*cos(w0)-2*sqrt(a)*alpha;
		break;
	};
	//first, the normalization and calculation of gain.
	/**Justification:
We have the transfer function h(z) = (b0+b1*Z^-1+b2*Z^-2)/(a0+a1*Z^-1+a2*z^-2).
Multiply the numerator by (1/b0) and the denominator by (1/a0); or the entire expression by ((1/b0)/(1/a0)).
Simplified in the usual manner, multiply by (a0/b0) to make the first terms 1 and avoid a multiplication.
But we want to undo this, so multiply by a gain that is the reciprocal:*/
	gain = b0/a0;
	//we then pull b0 out of b1 and b2, and a0 out of a1 and a2.
	b1 /= b0;
	b2 /= b0;
	a1 /= a0;
	a2 /= a0;
	//store on the class.
	this->gain = gain;
	this->a1 = a1;
	this->a2 = a2;
	this->b1 = b1;
	this->b2 = b2;
}


float LavBiquad::tick(float sample) {
	//broken up for sanity. No reason a good compiler won't optimize here.
	double term1 = gain*(sample + b1*history[1] + b2*history[0]);
	double term2 = a1*recursion_history[1] - a2*recursion_history[2];
	double result = term1-term2;
	//move the histories.
	history[0] = history[1];
	recursion_history[0] = recursion_history[1];
	history[1] = sample;
	recursion_history[1] = result;
	return (float)result;
}
