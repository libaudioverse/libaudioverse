/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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