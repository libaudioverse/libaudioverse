/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/constants.hpp"
#include <cmath>
#include <cfloat>
#include <stdio.h>

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
	void setPhase(float p);
	float tick();
	void reset();
	private:
	void recompute();
	double frequency = 100.0f, phase = 0.0f, phaseIncrement = 0.0f, sr = 0.0f, normFactor = 1.0f;
	int harmonics = 0, adjusted_harmonics = 0;
	bool shouldNormalize = false;
};

inline Blit::Blit(float _sr): sr(_sr) {
	recompute();
}

inline float Blit::tick() {
	double numer = std::sin(phase*(adjusted_harmonics+0.5));
	double denom = std::sin(phase/2.0);
	float res;
	if(std::abs(denom) < DBL_EPSILON) {
		//This is from Dodge and Jerse (1985), Computer Music: Synthesis, Composition, and Performance. 
		//It's probably a limit, but it wasn't worth me working through the math to find out.
		//res = (2*adjusted_harmonics+1)*cos(phase*(adjusted_harmonics+0.5))/cos(phase/2.0);
		res = adjusted_harmonics+1; //Derived by looking at what 1+2cos(x)+2cos(2x)... does at 0, pi, etc.
	}
	else res = (float)(numer/denom);
	phase += phaseIncrement;
	//Keep us from going over 2PI. 1 decrement with an if is not sufficient if we're aliasing.
	phase -= floorf(phase/(2*PI))*2*PI;
	return res*normFactor;
}

inline void Blit::reset() {
	phase = 0.0f;
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
}

inline void Blit::setShouldNormalize(bool norm) {
	shouldNormalize = norm;
	recompute();
}

inline void Blit::setPhase(float p) {
	phase = 2*PI*p;
}

}