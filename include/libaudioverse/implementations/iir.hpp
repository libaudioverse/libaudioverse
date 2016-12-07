/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once

namespace libaudioverse_implementation {

class IIRFilter {
	public:
	IIRFilter(double sr);
	float tick(float sample);
	void configure(int newNumeratorLength, double* newNumerator, int newDenominatorLength,  double* newDenominator);
	void setGain(double gain);
	void clearHistories();
	void configureBiquad(int type, double frequency, double dbGain, double q);
	double qFromBw(double frequency, double bw);
	double qFromS(double frequency, double s);
	
	private:
	double *history = nullptr, *recursion_history = nullptr, *numerator = nullptr, *denominator = nullptr;
	int numerator_length = 0, denominator_length = 0;
	double gain = 1.0;
	double sr;
};

}