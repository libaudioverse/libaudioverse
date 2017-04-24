/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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

void DoppleringDelayLine::setDelay(double d) {
	setDelayInSamples(d*sr);
}

void DoppleringDelayLine::setDelayInSamples(double newDelay) {
	counter = interpolation_time*sr;
	if(counter) {
		delta = (delay-newDelay)/counter;
	}
	delay = newDelay;
	if(slave) slave->setDelayInSamples(newDelay);
}

void DoppleringDelayLine::setInterpolationTime(float t) {
	interpolation_time = t;
	counter = 0;
	if(slave) slave->setInterpolationTime(t);
}

float DoppleringDelayLine::tick(float sample) {
	float retval = computeSample();
	advance(sample);
	return retval;
}

float DoppleringDelayLine::computeSample() {
	double targetDelay = delay+counter*delta;
	float w1 = targetDelay-floorf(targetDelay);
	float w2 = 1-w1;
	int i1 = (int)(targetDelay);
	int i2=i1+1;
	//make sure neither of these is over max delay.
	i1 =std::min(i1, max_delay);
	i2=std::min(i2, max_delay);
	return line.read(i1)*w1+line.read(i2)*w2;
}

void DoppleringDelayLine::advance(float sample) {
	line.advance(sample);
	if(counter) counter--;
}

void DoppleringDelayLine::process(int count, float* in, float* out) {
	int i = 0;
	// Take the slow path as long as we're crossfading.
	for(; i < count && counter; i++) out[i] = tick(in[i]);
	// then we can do this.
	float w1 = delay-floorf(delay);
	float w2 = 1-w1;
	int i1 = (int)(delay);
	int i2=i1+1;
	//make sure neither of these is over max delay.
	i1 =std::min(i1, max_delay);
	i2=std::min(i2, max_delay);
	for(; i < count; i++) {
		float sample = in[i];
		out[i] = w1*line.read(i1)+w2*line.read(i2);
		line.advance(sample);
	}
}

void DoppleringDelayLine::reset() {
	counter = 0;
	delta = 0.0;
	line.reset();
	if(slave) slave->reset();
}

DoppleringDelayLine* DoppleringDelayLine::getSlave() {
	return slave;
}

void DoppleringDelayLine::setSlave(DoppleringDelayLine* s) {
	slave = s;
}

}