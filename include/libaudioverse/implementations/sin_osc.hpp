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

Resync is the number of samples after which we resynchronize the oscillator; 64 is usually sufficient.
*/

class SinOsc {
	public:
	SinOsc(float _sr, int _resync = 64): sr(_sr), resync(_resync), resyncCounter(_resync) {}
	
	double tick() {
		//We have to return what we are now, so that the first tick is at phase zero instead of zero and a bit.
		double res = sx;
		double nsx = sx*cd+cx*sd;
		double ncx = cx*cd-sx*sd;
		phase += phaseIncrement;
		if(phase > phaseWrap) {
			int m = phase/phaseWrap;
			phase = phase-m*phaseWrap;
			doResync();
		}
		resyncCounter --;
		if(resyncCounter == 0) doResync();
		else {
			sx = nsx;
			cx = ncx;
		}
		return res;
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
	
	//Usually this is 2PI, but things like the blit need custom configurations.
	void setPhaseWrap(double p) {
		phaseWrap = p*2*PI;
	}
	
	private:
	//s=sin, c=cos
	//internal vector is right, frequency is zero.
	double sx = 0, cx = 1, sd = 0, cd = 0;
	float sr; //sampling rate.
	//frequency is saved for purposes of skipping samples.
	double frequency =0;
	double phase = 0, phaseIncrement = 0, phaseWrap = 2*PI;
	int resync, resyncCounter;
};

}