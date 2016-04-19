/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "sin_osc.hpp"
#include "../private/constants.hpp"
#include <vector>

namespace libaudioverse_implementation {

/**An additive square wave.
This is the most perfect and slowest method of making square waves, with absolutely no error.

The formula for a square wave is as follows:
sin(f)+sin(3f)/3+sin(5f)/5...
Where sin denotes sine waves.
*/

class AdditiveSquare {
	public:
	AdditiveSquare(float _sr);
	double tick();
	void reset();
	void setFrequency(float frequency);
	float getFrequency();
	void setPhase(double phase);
	double getPhase();
	//0 means autoadjust.
	void setHarmonics(int harmonics);
	int getHarmonics();
	private:
	void readjustHarmonics();
	std::vector<SinOsc> oscillators;
	SinOsc* oscillators_array; //Avoid taking the address of the first item of the vector repeatedly.
	int harmonics = 0, adjusted_harmonics = 0;
	float frequency = 100;
	float sr;
	double normFactor = 1.0;
};

inline AdditiveSquare::AdditiveSquare(float _sr): sr(_sr) {
	//These trigger the recomputation logic.
	setHarmonics(0);
	setFrequency(100);
	readjustHarmonics();
}

inline double AdditiveSquare::tick() {
	double sum = 0.0;
	for(int i = 0; i < adjusted_harmonics; i++) sum += oscillators_array[i].tick()/(2*(i+1)-1);
	return sum*normFactor;
}

inline void AdditiveSquare::reset() {
	for(auto &i: oscillators) i.reset();
}

inline void AdditiveSquare::setFrequency(float frequency) {
	this->frequency = frequency;
	readjustHarmonics();
	for(int i = 0; i < adjusted_harmonics; i++) {
		oscillators_array[i].setFrequency(frequency*(2*(i+1)-1));
	}
	//We force the phase to be what we need, so that the higher harmonics align properly.
	setPhase(getPhase());
}

inline float AdditiveSquare::getFrequency() {
	return frequency;
}

inline void AdditiveSquare::setPhase(double phase) {
	for(int i = 0; i < adjusted_harmonics; i++) oscillators_array[i].setPhase((2*(i+1)-1)*phase);
}

inline double AdditiveSquare::getPhase() {
	return oscillators_array[0].getPhase();
}

inline void AdditiveSquare::setHarmonics(int harmonics) {
	this->harmonics = harmonics;
	readjustHarmonics();
}

inline int AdditiveSquare::getHarmonics() {
	return harmonics;
}

inline void AdditiveSquare::readjustHarmonics() {
	int newHarmonics;
	if(harmonics == 0) {
		//Number of harmonics we can get between 0 and nyquist.
		double nyquist = sr/2.0;
		double range = nyquist-frequency;
		newHarmonics = 1+range/(2*frequency);
	}
	else newHarmonics = harmonics;
	oscillators.resize(newHarmonics, SinOsc(sr));
	oscillators_array = &oscillators[0];
	//Partial setPhase.
	double p = getPhase();
	for(int i = adjusted_harmonics; i < newHarmonics; i++) oscillators_array[i].setPhase((2*(i+1)-1)*p);
	adjusted_harmonics = newHarmonics;
	//4/PI comes from the Wikipedia definition of square wave. The second constant accounts for the Gibbs phenomenon.
	//The final term was derived experimentally, by figuring out what the maximum and minimum look like.
	//Without it, we overshoot very slightly, which is worse than undershooting very slightly.
	if(adjusted_harmonics > 1) normFactor = (4.0/PI)*(1.0/(1.0+2.0*WILBRAHAM_GIBBS))*(1.0/1.01);
	//Otherwise, we have 1 harmonics, and that's a trivial case.
	else normFactor = 1.0;
}

}