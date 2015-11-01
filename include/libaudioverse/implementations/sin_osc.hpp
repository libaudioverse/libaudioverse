/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <math.h>
#include "../private/constants.hpp"

namespace libaudioverse_implementation {

/*A fast sine oscillator.

The following algorithm is based off the angle sum and difference identities.
This is also equivalent to rotating a unit vector by f/sr radians every tick.

Trig identities:
sin(x+d)=sin(x)cos(d)+cos(x)sin(d)
cos(x+d)=cos(x)cos(d)-sin(x)sin(d)	

We have to use double, or the oscillator becomes erronious within only a few samples.

Resync is the number of samples after which we resynchronize the oscillator; 100 is usually sufficient.
*/

class SinOsc {
	public:
	SinOsc(float _sr, int _resync = 100): sr(_sr), resync(_resync), resyncCounter(_resync) {}
	
	double tick() {
		double ocx=cx, osx=sx;
		sx = osx*cd+ocx*sd;
		cx = ocx*cd-osx*sd;
		return sx;
		phase += phaseIncrement;
		if(phase > 2*PI) phase -= 2*PI;
		resyncCounter --;
		if(resyncCounter == 0) doResync();
	}

	void doResync() {
		sx = sin(phase);
		cx = cos(phase);
		resyncCounter = resync;
	}
	
	//Skips count samples.
	//Same as calling tick count times.
	void skipSamples(int count) {
		phase += phaseIncrement*count;
		resyncCounter = resync;
		cx=cos(phase);
		sx=sin(phase);
	}

	//Set the phase increment per sample.
	void setPhaseIncrement(double i) {
		phaseIncrement = 2*PI*i;
		cd = cos(phaseIncrement);
		sd = sin(phaseIncrement);
	}
	
	void setFrequency(float f) {
		frequency = f;
		setPhaseIncrement(f/sr);
	}

	void reset() {
		//point the internal vector at the positive x axis, also known as phase 0.
		sx=0;
		cx =1;
		sd=sin(phaseIncrement);
		cd=cos(phaseIncrement);
	}
	
	//phase is from 0 to 1 and measured in  periods.
	void setPhase(double phase) {
		cx = cos(2*PI*phase);
		sx = sin(2*PI*phase);
		resyncCounter = resync;
	}
	
	double getPhase() {
		return phase;
	}
	
	private:
	//s=sin, c=cos
	//internal vector is right, frequency is zero.
	double sx = 0, cx = 1, sd = 0, cd = 0;
	float sr; //sampling rate.
	//frequency is saved for purposes of skipping samples.
	double frequency =0;
	double phase = 0, phaseIncrement = 0;
	int resync, resyncCounter;
};

}