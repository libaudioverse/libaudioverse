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

/**An additive sawtooth wave.

The formula for a sawtooth wave is as follows:

0.5-(1/pi)*(sin(x)+sin(2x)/2+sin(3x)/3+sin(4x)/4...)
This ranges from 0 to 1. When we subtract 0.5 and multiply by 2, the first term vanishes.
*/

class AdditiveSaw {
	public:
	AdditiveSaw(float _sr);
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

inline AdditiveSaw::AdditiveSaw(float _sr): sr(_sr) {
	//These trigger the recomputation logic.
	setHarmonics(0);
	setFrequency(100);
	readjustHarmonics();
}

inline double AdditiveSaw::tick() {
	double sum = 0.0;
	for(int i = 0; i != adjusted_harmonics; i++) sum -= oscillators_array[i].tick()/(i+1);
	return sum*normFactor;
}

inline void AdditiveSaw::reset() {
	for(auto &i: oscillators) i.reset();
}

inline void AdditiveSaw::setFrequency(float frequency) {
	this->frequency = frequency;
	readjustHarmonics();
	for(int i = 0; i < adjusted_harmonics; i++) {
		oscillators_array[i].setFrequency(frequency*(i+1));
	}
	//We force the phase to be what we need, so that the higher harmonics align properly.
	setPhase(getPhase());
}

inline float AdditiveSaw::getFrequency() {
	return frequency;
}

inline void AdditiveSaw::setPhase(double phase) {
	for(int i = 0; i < adjusted_harmonics; i++) oscillators_array[i].setPhase((i+1)*phase);
}

inline double AdditiveSaw::getPhase() {
	return oscillators_array[0].getPhase();
}

inline void AdditiveSaw::setHarmonics(int harmonics) {
	this->harmonics = harmonics;
	readjustHarmonics();
}

inline int AdditiveSaw::getHarmonics() {
	return harmonics;
}

inline void AdditiveSaw::readjustHarmonics() {
	int newHarmonics;
	if(harmonics == 0) {
		//Number of harmonics we can get between 0 and nyquist.
		double nyquist = sr/2.0;
		newHarmonics = nyquist/frequency;
		if(newHarmonics == 0) newHarmonics = 1;
	}
	else newHarmonics = harmonics;
	oscillators.resize(newHarmonics, SinOsc(sr));
	oscillators_array = &oscillators[0];
	//Partial setPhase.
	double p = getPhase();
	for(int i = adjusted_harmonics; i < newHarmonics; i++) oscillators_array[i].setPhase((i+1)*p);
	adjusted_harmonics = newHarmonics;
	//Note that we are actually on the range -0.5 to 0.5, until fixed by normFactor.
	if(adjusted_harmonics > 1) normFactor = 2*(1.0/(1.0+2*WILBRAHAM_GIBBS))*(1/PI);
	//Otherwise, we have 1 harmonics, and that's a trivial case.
	else normFactor = 1.0;
}

}