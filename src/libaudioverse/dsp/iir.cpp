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
	if(history) delete[] history;
	if(numerator) delete[] numerator;
	if(denominator) delete[] denominator;
	if(recursion_history) delete[] recursion_history;
	if(numerator_length == 0 || denominator_length == 0) throw LavErrorException(Lav_ERROR_RANGE); //each must ahve at least one sample.
	history = new double[newNumeratorLength]();
	recursion_history = new double[newDenominatorLength]();
	numerator = new double[newNumeratorLength]();
	denominator = new double[newDenominatorLength]();
	std::copy(newNumerator, newNumerator+newNumeratorLength, numerator);
	std::copy(newDenominator, newDenominator+newDenominatorLength, denominator);
	numerator_length= newNumeratorLength;
	denominator_length = newDenominatorLength;
}

void LavIIRFilter::clearHistories() {
	if(numerator_length) memset(numerator, 0, sizeof(double)*numerator_length);
	if(denominator_length) memset(denominator, 0, sizeof(double)*denominator_length);
}

float LavIIRFilter::tick(float sample) {
	//roll back the history.
	std::copy(history+1, history+numerator_length, history);
	//put sample in the rightmost history slot.
	history[numerator_length-1] = sample;
	//result.
	double result = 0.0;
	//first, a convolution but with doubles.  Convolve the numerator coefficients with the history, adding terms.
	for(int i = 0; i < numerator_length; i++) result+=history[numerator_length-i-1]*numerator[i];
	//second, do likewise with recursion history, but this time subtract.
	for(int i = 0; i < denominator_length; i++) result-=recursion_history[denominator_length-i-1]*denominator[i];
	//roll back the recursion history.
	std::copy(recursion_history+1, recursion_history+denominator_length, recursion_history);
	//insert result into the recursion history.
	recursion_history[denominator_length-1] = result;
	return (float)result;
}
