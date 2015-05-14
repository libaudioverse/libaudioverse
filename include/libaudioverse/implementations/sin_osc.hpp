/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <math.h>
#include "../private/constants.hpp"

namespace libaudioverse_implementation {

//A fast sine oscillator with renormalizing capabilities.
//This is in the header because we want inlining.
//The following algorithm is based off the angle sum and difference identities.
//This is also equivalent to rotating a unit vector by f/sr radians every tick.
class SinOsc {
	public:
	SinOsc(float _sr): sr(_sr) {}
	float tick() {
		/*Trig identities:
		sin(x+d)=sin(x)cos(d)+cos(x)sin(d)
		cos(x+d)=cos(x)cos(d)-sin(x)sin(d)
		*/
		float ocx=cx, osx=sx;
		sx = osx*cd+ocx*sd;
		cx = ocx*cd-osx*sd;
		return osx;
	}
	void setFrequency(float f) {
		//We don't lose phase, so don't touch cx and sx.
		sd= sinf(2*PI*f/sr);
		cd=cosf(2*PI*f/sr);
	}
	void normalize() {
		float magnitude=sqrtf(cx*cx+sx*sx);
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
	private:
	//s=sin, c=cos
	//internal vector is right, frequency is zero.
	float sx = 0, cx = 1, sd = 0, cd = 0;
	float sr; //sampling rate.
};

}