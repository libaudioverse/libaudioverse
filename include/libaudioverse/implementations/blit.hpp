/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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
	float tick();
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

inline float Blit::tick() {
	double numer = numerOsc.tick();
	double denom = denomOsc.tick();
	float res;
	//Note that the oscillators are only ever "perfect" at the beginning and immediately after a resync.
	//Therefore we have to allow for some leeway here.
	//The following number was found by determining the error on a sine node to be about 1e-4, and then choosing something larger than it.
	if(std::abs(denom) < 1e-3) {
		//This is from Dodge and Jerse (1985), Computer Music: Synthesis, Composition, and Performance. 
		//It's probably a limit, but it wasn't worth me working through the math to find out.
		double p = 2*PI*phase;
		res = (2*adjusted_harmonics+1)*cos(p*(adjusted_harmonics+0.5))/cos(p/2.0);
	}
	else res = (float)(numer/denom);
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
		//If we divided the sampling rate evenly, we have a harmonic at nyquist.
		//This would be fine, but the numerator of the above function is then technically aliasing.
		if(adjusted_harmonics*sr >= sr/2) adjusted_harmonics -= 1;
		//We have a harmonic at dc, always.  The rest of the harmonics want to cancel in pairs. To that end, we need an odd adjuisted harmonics for the best approximation.
		if(adjusted_harmonics%2 == 0) adjusted_harmonics -= 1;
		//But we need at least one.
		if(adjusted_harmonics <= 0) adjusted_harmonics = 1;
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