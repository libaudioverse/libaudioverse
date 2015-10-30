/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/constants.hpp"
#include <cmath>
#include <cfloat>
#include <limits>

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
	float frequency = 100.0f, phase = 0.0f, phaseIncrement = 0.0f, sr = 0.0f, normFactor = 1.0f;
	int harmonics = 0, adjusted_harmonics = 0;
	bool shouldNormalize = false;
};

inline Blit::Blit(float _sr): sr(_sr) {
	recompute();
}

inline float Blit::tick() {
	double numer = std::sin(phase*(harmonics+1)/2.0);
	double denom = std::sin(phase/2);
	phase += phaseIncrement;
	//Keep us from going over 2PI. 1 decrement with an if is not sufficient if we're aliasing.
	phase -= floorf(phase/(2*PI))*2*PI;
	float res;
	if(std::abs(denom) < DBL_EPSILON) res = (float)(2*harmonics+1);
	else res = (float)(numer/denom);
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
		harmonics = floor(sr/frequency);
		if(harmonics == 0) harmonics = 1;
	}
	phaseIncrement = 2*PI*frequency/sr;
	if(shouldNormalize) normFactor = 1.0f/harmonics;
	else normFactor = 1.0f;
}

inline void Blit::setShouldNormalize(bool norm) {
	shouldNormalize = norm;
	recompute();
}

inline void Blit::setPhase(float p) {
	phase = p;
}

}