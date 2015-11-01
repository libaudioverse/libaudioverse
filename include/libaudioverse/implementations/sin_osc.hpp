/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <math.h>
#include "../private/constants.hpp"

namespace libaudioverse_implementation {

/*A fast sine oscillator with renormalizing capabilities.

The following algorithm is based off the angle sum and difference identities.
This is also equivalent to rotating a unit vector by f/sr radians every tick.

Trig identities:
sin(x+d)=sin(x)cos(d)+cos(x)sin(d)
cos(x+d)=cos(x)cos(d)-sin(x)sin(d)	

We have to use double, or the oscillator becomes erronious within only a few samples.
*/

class SinOsc {
	public:
	SinOsc(float _sr): sr(_sr) {}
	
	double tick() {
		double ocx=cx, osx=sx;
		sx = osx*cd+ocx*sd;
		cx = ocx*cd-osx*sd;
		return sx;
	}

	//Skips count samples.
	//Same as calling tick count times.
	void skipSamples(int count) {
		//First, our angle may be derived from sx and cx:
		float angle=atan2(sx, cx);
		//The amount to advance by in radians.
		//Compute periods, multiply by 2PI.
		float advanceBy = (count/sr)*frequency*2*PI;
		angle+=advanceBy;
		cx=cos(angle);
		sx=sin(angle);
	}

	//Set the phase increment per sample.
	void setPhaseIncrement(double i) {
		i = 2*PI*i;
		cd = cos(i);
		sd = sin(i);
	}
	
	void setFrequency(float f) {
		frequency = f;
		setPhaseIncrement(f/sr);
	}
	
	void normalize() {
		double magnitude=sqrt(cx*cx+sx*sx);
		sx/= magnitude;
		cx /= magnitude;
	}
	
	void reset() {
		//point the internal vector at the positive x axis, also known as phase 0.
		sx=0;
		cx =1;
		sd=0;
		cd=0;
	}
	
	//phase is from 0 to 1 and measured in  periods.
	void setPhase(double phase) {
		cx = cos(2*PI*phase);
		sx = sin(2*PI*phase);
	}
	
	double getPhase() {
		return atan2(sx, cx)/(2*PI);
	}
	
	private:
	//s=sin, c=cos
	//internal vector is right, frequency is zero.
	double sx = 0, cx = 1, sd = 0, cd = 0;
	float sr; //sampling rate.
	//frequency is saved for purposes of skipping samples.
	double frequency =0;
};

}