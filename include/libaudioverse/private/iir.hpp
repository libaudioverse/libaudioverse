/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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