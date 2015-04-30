/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implement a generic IIR filter.*/
#include <algorithm>
#include <libaudioverse/private/iir.hpp>
//for biquad types.
#include <libaudioverse/libaudioverse_properties.h>
//for error codes
#include <libaudioverse/libaudioverse.h>
#include <math.h>
#include <libaudioverse/private/constants.hpp>
#include <stdio.h>
#include <libaudioverse/private/errors.hpp>

void IIRFilter::configure(int newNumeratorLength, double* newNumerator, int newDenominatorLength, double* newDenominator) {
	if(newNumeratorLength == 0 || newDenominatorLength == 0) throw LavErrorException(Lav_ERROR_RANGE);
	//we normalize by the first coefficient but throw it out; consequently, it must be nonzero.
	if(newDenominator[0] == 0.0) throw LavErrorException(Lav_ERROR_RANGE);
	if(numerator_length != newNumeratorLength || denominator_length != newDenominatorLength) { //only clear these if absolutely necessary.
		if(history) delete[] history;
		if(numerator) delete[] numerator;
		if(denominator) delete[] denominator;
		if(recursion_history) delete[] recursion_history;
		history = new double[newNumeratorLength]();
		recursion_history = new double[newDenominatorLength]();
		numerator = new double[newNumeratorLength]();
		denominator = new double[newDenominatorLength]();
	}
	std::copy(newNumerator, newNumerator+newNumeratorLength, numerator);
	std::copy(newDenominator, newDenominator+newDenominatorLength, denominator);
	numerator_length= newNumeratorLength;
	denominator_length = newDenominatorLength;
	for(int i = 0; i <denominator_length; i++) denominator[i]/=denominator[0];
}

void IIRFilter::clearHistories() {
	if(numerator_length) memset(history, 0, sizeof(double)*numerator_length);
	if(denominator_length) memset(recursion_history, 0, sizeof(double)*denominator_length);
}

void IIRFilter::setGain(double gain) {
	this->gain = gain;
}

float IIRFilter::tick(float sample) {
	int i;
	history[0] = sample*gain;
	recursion_history[0] = 0.0;
	for(i= numerator_length-1; i > 0; i--) {
		recursion_history[0]+=history[i]*numerator[i];
		history[i]=history[i-1];
	}
	recursion_history[0]+=history[0]*numerator[0];
	for(i = denominator_length-1 ; i >0; i--) {
		recursion_history[0]+=-denominator[i]*recursion_history[i];
		recursion_history[i]=recursion_history[i-1];
	}
	return (float)recursion_history[0];
}

void IIRFilter::configureBiquad(int type, double sr, double frequency, double dbGain, double q) {
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
		case Lav_BIQUAD_TYPE_IDENTITY:
		b0 = 1;
		b1 = 0;
		b2 = 0;
		a0 = 1;
		a1 = 0;
		a2 = 0;
		break;
	};
	double numerator[] = {1, b1/b0, b2/b0};
	double denominator[] = {1, a1/a0, a2/a0};
	configure(3, numerator, 3, denominator);
	setGain(b0/a0);
}
