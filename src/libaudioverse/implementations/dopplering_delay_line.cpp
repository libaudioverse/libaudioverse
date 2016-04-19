/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

DoppleringDelayLine::DoppleringDelayLine(float maxDelay, float sr): line((int)(sr*maxDelay)+1) {
	this->sr = sr;
	max_delay = (int)(sr*maxDelay)+1;
}

void DoppleringDelayLine::setDelay(float d) {
	//stop if we're crossfading.
	if(counter) {
		delay = new_delay;
		counter = 0;
	}
	new_delay = d*sr;
	counter = interpolation_time*sr;
	if(counter == 0) counter=1;
	if(sr*interpolation_time !=0.0) velocity = (new_delay-delay)/(sr*interpolation_time);
	else velocity = 0.0;
	if(slave) slave->setDelay(d);
}

void DoppleringDelayLine::setDelayInSamples(int newDelay) {
	//We change newDelay, so send it along first.
	if(slave) slave->setDelayInSamples(newDelay);
	if(counter) {
		delay = new_delay;
		counter = 0;
	}
	new_delay = std::min(newDelay, max_delay);
	counter = interpolation_time*sr;
	if(counter == 0) counter=1;
	if(sr*interpolation_time !=0.0) velocity = (new_delay-delay)/(sr*interpolation_time);
	else velocity = 0.0;
}

void DoppleringDelayLine::setInterpolationTime(float t) {
	interpolation_time = t;
	if(slave) slave->setInterpolationTime(t);
}

float DoppleringDelayLine::tick(float sample) {
	float retval = computeSample();
	advance(sample);
	return retval;
}

float DoppleringDelayLine::computeSample() {
	float w1 = delay-floorf(delay);
	float w2 = 1-w1;
	int i1 = (int)(delay);
	int i2=i1+1;
	//make sure neither of these is over max delay.
	i1 =std::min(i1, max_delay);
	i2=std::min(i2, max_delay);
	return line.read(i1)*w1+line.read(i2)*w2;
}

void DoppleringDelayLine::advance(float sample) {
	line.advance(sample);
	if(counter) {
		delay += velocity;
		counter--;
		if(counter ==0) delay = new_delay; //make sure it's perfect.
	}
}

void DoppleringDelayLine::reset() {
	velocity = 0.0;
	if(counter) {
		delay=new_delay;
		counter=0;
	}
	line.reset();
}

DoppleringDelayLine* DoppleringDelayLine::getSlave() {
	return slave;
}

void DoppleringDelayLine::setSlave(DoppleringDelayLine* s) {
	slave = s;
}

}