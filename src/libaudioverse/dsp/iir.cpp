/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implement a generic IIR filter.*/
#include <algorithm>
#include <libaudioverse/private_iir.hpp>
#include <libaudioverse/libaudioverse_properties.h>
#include <math.h>
#include <libaudioverse/private_constants.hpp>
#include <stdio.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_errors.hpp>

void LavIIRFilter::configure(int newNumeratorLength, double* newNumerator, int newDenominatorLength, double* newDenominator) {
	if(newNumeratorLength == 0 || newDenominatorLength == 0 || newDenominator[0] == 0.0) throw LavErrorException(Lav_ERROR_RANGE); //each must have at least one sample, and a0==0 is a pole at the origin which can be done with the mul property or similar.
	if(history) delete[] history;
	if(numerator) delete[] numerator;
	if(denominator) delete[] denominator;
	if(recursion_history) delete[] recursion_history;
	history = new double[newNumeratorLength]();
	recursion_history = new double[newDenominatorLength]();
	numerator = new double[newNumeratorLength]();
	denominator = new double[newDenominatorLength]();
	std::copy(newNumerator, newNumerator+newNumeratorLength, numerator);
	std::copy(newDenominator, newDenominator+newDenominatorLength, denominator);
	numerator_length= newNumeratorLength;
	denominator_length = newDenominatorLength;
	//we normalize by a0.
	for(int i = 0; i < newDenominatorLength; i++) newDenominator[i]/=newDenominator[0];
}

void LavIIRFilter::clearHistories() {
	if(numerator_length) memset(history, 0, sizeof(double)*numerator_length);
	if(denominator_length) memset(recursion_history, 0, sizeof(double)*denominator_length);
}

float LavIIRFilter::tick(float sample) {
	int i;
	double result = 0.0;
	history[0] = sample;
	for(i= numerator_length-1; i > 0; i--) {
		result +=history[i]*numerator[i];
		history[i]=history[i-1];
	}
	result+=sample*numerator[0];
	for(i = denominator_length-1 ; i >0; i--) {
		result +=-denominator[i]*recursion_history[i];
		recursion_history[i]=recursion_history[i-1];
	}
	result += -denominator[0]*recursion_history[0];
	recursion_history[0]=result;
	return (float)result;
}
