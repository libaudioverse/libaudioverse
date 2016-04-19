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

/**An additive triangle wave.

The formula for a triangle wave is as follows:
sin(f)-sin(3f)/9+sin(5f)/25-sin(7f)/49...
Where sin denotes sine waves.
*/

class AdditiveTriangle {
	public:
	AdditiveTriangle(float _sr);
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

inline AdditiveTriangle::AdditiveTriangle(float _sr): sr(_sr) {
	//These trigger the recomputation logic.
	setHarmonics(0);
	setFrequency(100);
	readjustHarmonics();
}

inline double AdditiveTriangle::tick() {
	double sum = 0.0;
	int h;
	//Get the ones we add first.
	for(int i = 0; i < adjusted_harmonics; i += 2) {
		h = 2*i+1;
		sum += oscillators_array[i].tick()/(h*h);
	}
	//And subtract.
	for(int i = 1; i < adjusted_harmonics; i+=2) {
		h = 2*i+1;
		sum -= oscillators_array[i].tick()/(h*h);
	}
	return sum*normFactor;
}

inline void AdditiveTriangle::reset() {
	for(auto &i: oscillators) i.reset();
}

inline void AdditiveTriangle::setFrequency(float frequency) {
	this->frequency = frequency;
	readjustHarmonics();
	for(int i = 0; i < adjusted_harmonics; i++) {
		oscillators_array[i].setFrequency(frequency*(2*(i+1)-1));
	}
	//We force the phase to be what we need, so that the higher harmonics align properly.
	setPhase(getPhase());
}

inline float AdditiveTriangle::getFrequency() {
	return frequency;
}

inline void AdditiveTriangle::setPhase(double phase) {
	for(int i = 0; i < adjusted_harmonics; i++) oscillators_array[i].setPhase((2*(i+1)-1)*phase);
}

inline double AdditiveTriangle::getPhase() {
	return oscillators_array[0].getPhase();
}

inline void AdditiveTriangle::setHarmonics(int harmonics) {
	this->harmonics = harmonics;
	readjustHarmonics();
}

inline int AdditiveTriangle::getHarmonics() {
	return harmonics;
}

inline void AdditiveTriangle::readjustHarmonics() {
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
	//(78/PI^2) comes from the Wikipedia definition of triangle wave.
	//Note that triangle waves do not have Gibbs Phenomenon.
	if(adjusted_harmonics > 1) normFactor = 8.0/(PI*PI);
	//Otherwise, we have 1 harmonics, and that's a trivial case.
	else normFactor = 1.0;
}

}