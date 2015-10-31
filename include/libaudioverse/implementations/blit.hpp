/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/constants.hpp"
#include "sin_osc.hpp"
#include <cmath>
#include <cfloat>


namespace libaudioverse_implementation {

/**A bandlimited impulse train with configurable harmonics.

A number of interesting waveforms can be constructed from this bandlimited impulse train, most notably triangle and square.
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
	//These oscillators are set up so that we don't have to use the trig functions.
	SinOsc numerOsc, denomOsc;
};

inline Blit::Blit(float _sr): sr(_sr), numerOsc(_sr), denomOsc(_sr) {
	recompute();
}

inline float Blit::tick() {
	double numer = numerOsc.tick();
	double denom = denomOsc.tick();
	float res;
	if(std::abs(denom) < DBL_EPSILON) {
		res = adjusted_harmonics+1; //Derived by looking at what 1+2cos(x)+2cos(2x)... does at 0, pi, etc.
	}
	else res = (float)(numer/denom);
	phase += phaseIncrement;
	if(phase >= 2*PI) phase -= 2*PI;
	return res*normFactor;
}

inline void Blit::reset() {
	phase = 0.0f;
	numerOsc.reset();
	denomOsc.reset();
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
	phaseIncrement = 2*PI*frequency/sr;
	//1+2cos(x)+2cos(2x)...etc...means max value of 1+2*adjusted_harmonics.
	if(shouldNormalize) normFactor = 1.0f/(2*adjusted_harmonics+1);
	//Fourier series of unnormalized BLIT has 1/period=frequency coefficient.
	else normFactor = frequency;
	//Configure the oscillators.
	//Numerator has a phase increment of (adjusted_harmonics+0.5)*phaseIncrement, but these take it on the range 0...1.
	numerOsc.setPhaseIncrement(phaseIncrement*(adjusted_harmonics+0.5)/(2*PI));
	//Denominator's value increments by phaseIncrement/2.
	denomOsc.setPhaseIncrement(phaseIncrement/(4*PI));
	//The phases are also on the range 0...1, and can be computred similarly.
	numerOsc.setPhase(phase*(adjusted_harmonics+0.5)/(2*PI));
	denomOsc.setPhase(phase/(4*PI));
}

inline void Blit::setShouldNormalize(bool norm) {
	shouldNormalize = norm;
	recompute();
}

inline void Blit::setPhase(double p) {
	//If phase is wrapping, deal with it.
	phase = 2*PI*(p-floor(p));
	recompute();
}

inline double Blit::getPhase() {
	return phase/(2*PI);
}

}