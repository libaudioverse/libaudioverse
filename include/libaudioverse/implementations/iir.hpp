/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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