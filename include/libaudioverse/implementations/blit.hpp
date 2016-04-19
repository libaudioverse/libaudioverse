/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "sin_osc.hpp"
#include "../private/constants.hpp"
#include <cmath>
#include <cfloat>
#include <stdio.h>

namespace libaudioverse_implementation {

/**A bandlimited impulse train with configurable harmonics.

A number of interesting waveforms can be constructed from this bandlimited impulse train, most notably triangle and square.

This class is based off the following trig identity, but optimized to hell and using fast sine oscillators:
1+2cos(x)+2cos(2x)+...+2cos(nx)=sin((n+0.5) phase)/sin(phase / 2)
*/
class Blit {
	public:
	Blit(float sr);
	void setFrequency(float frequency);
	void setHarmonics(int harmonics); //If zero, adjust to get as many as possible.
	void setShouldNormalize(bool norm);
	void setPhase(double p);
	double getPhase();
	double tick();
	void reset();
	private:
	void recompute();
	double frequency = 100.0f, phase = 0.0f, phaseIncrement = 0.0f, sr = 0.0f, normFactor = 1.0f;
	int harmonics = 0, adjusted_harmonics = 0;
	bool shouldNormalize = false;
	SinOsc numerOsc, denomOsc;
};

inline Blit::Blit(float _sr): sr(_sr), numerOsc(_sr), denomOsc(_sr) {
	recompute();
}

inline double Blit::tick() {
	double numer = numerOsc.tick();
	double denom = denomOsc.tick();
	double res;
	//Note that the oscillators are only ever "perfect" at the beginning and immediately after a resync.
	//Therefore we have to allow for some leeway here.
	//The following number was found by determining the error on a sine node to be about 1e-4, and then experimenting.
	//If this is too large, then the formula will bounce between 0 and pi at the beginning of every cycle.
	if(std::abs(denom) < 1e-6) {
		//This is from Dodge and Jerse (1985), Computer Music: Synthesis, Composition, and Performance. 
		//It's probably a limit, but it wasn't worth me working through the math to find out.
		double p = 2*PI*phase;
		double nc = cos(p*(adjusted_harmonics+0.5));
		double dc = cos(p/2);
		res = (2*adjusted_harmonics+1)*nc/dc;
	}
	else res = numer/denom;
	phase += phaseIncrement;
	phase -= floorf(phase);
	return res*normFactor;
}

inline void Blit::reset() {
	setPhase(0.0);
}

inline void Blit::setHarmonics(int harmonics) {
	this->harmonics = harmonics;
	recompute();
}

inline void Blit::setFrequency(float frequency) {
	this->frequency = frequency;
	recompute();
}

inline void Blit::recompute() {
	if(harmonics == 0) {
		adjusted_harmonics = floor((sr/2)/frequency);
		//Keep the numerator from aliasing.
		if((adjusted_harmonics+0.5)*sr >= sr/2) adjusted_harmonics -= 1;
		//The formula does not break for harmonics = 0.
		if(adjusted_harmonics < 0) adjusted_harmonics = 0;
	}
	else adjusted_harmonics = harmonics;
	phaseIncrement = frequency/sr;
	//1+2cos(x)+2cos(2x)...etc...means max value of 1+2*adjusted_harmonics.
	if(shouldNormalize) normFactor = 1.0f/(2*adjusted_harmonics+1);
	//Fourier series of unnormalized BLIT has 1/period=frequency coefficient.
	else normFactor = frequency;
	//Set up the oscillators.
	numerOsc.setPhaseWrap(adjusted_harmonics+0.5);
	denomOsc.setPhaseWrap(0.5);
	numerOsc.setPhase(phase*(adjusted_harmonics+0.5));
	denomOsc.setPhase(0.5*phase);
	numerOsc.setPhaseIncrement((adjusted_harmonics+0.5)*phaseIncrement);
	denomOsc.setPhaseIncrement(phaseIncrement*0.5);
}

inline void Blit::setShouldNormalize(bool norm) {
	shouldNormalize = norm;
	recompute();
}

inline void Blit::setPhase(double p) {
	//If phase is wrapping, deal with it.
	phase = p-floor(p);
}

inline double Blit::getPhase() {
	return phase/(2*PI);
}

}