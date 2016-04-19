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

InterpolatedDelayLine::InterpolatedDelayLine(float maxDelay, float sr): line((int)(sr*maxDelay)+1) {
	this->sr = sr;
	max_delay = (int)(sr*maxDelay)+1;
}

void InterpolatedDelayLine::setDelay(float d) {
	delay = d*sr;
	if(slave) slave->setDelay(d);
}

void InterpolatedDelayLine::setDelayInSamples(int samples) {
	delay = std::min(samples, max_delay);
	if(slave) slave->setDelayInSamples(samples);
}

float InterpolatedDelayLine::tick(float sample) {
	float retval = computeSample();
	advance(sample);
	return retval;
}

float InterpolatedDelayLine::computeSample() {
	float w1 = delay-floorf(delay);
	float w2 = 1-w1;
	int i1 = (int)(delay);
	int i2=i1+1;
	//make sure neither of these is over max delay.
	i1 =std::min(i1, max_delay);
	i2=std::min(i2, max_delay);
	return line.read(i1)*w1+line.read(i2)*w2;
}

void InterpolatedDelayLine::advance(float sample) {
	line.advance(sample);
}

void InterpolatedDelayLine::reset() {
	line.reset();
}

InterpolatedDelayLine* InterpolatedDelayLine::getSlave() {
	return slave;
}

void InterpolatedDelayLine::setSlave(InterpolatedDelayLine* s) {
	slave = s;
}

}