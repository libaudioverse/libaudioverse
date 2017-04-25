/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

DoppleringDelayLine::DoppleringDelayLine(float maxDelay, float sr): line((int)(sr*maxDelay)+1) {
	this->sr = sr;
	max_delay = (int)(sr*maxDelay)+1;
}

void DoppleringDelayLine::setDelay(double d, bool shouldCrossfade) {
	setDelayInSamples(d*sr, shouldCrossfade);
}

void DoppleringDelayLine::setDelayInSamples(double newDelay, bool shouldCrossfade) {
	if(shouldCrossfade) {
		counter = interpolation_time*sr;
		if(counter) {
			delta = (delay-newDelay)/counter;
		}
	}
	else counter = 0;
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
	float last = line.read(i2);
	unsigned int iterationSize = 128;
	float tmp[128];
	// Put a lower bound on this.
	while(iterationSize > 8) {
		while(count-i > iterationSize) {
			std::copy(in+i, in+i+iterationSize, tmp);
			line.process(i1, iterationSize, tmp, tmp);
			// We can use scalarMultiplicationKernel with a bit of creativity.
			scalarMultiplicationKernel(iterationSize, w1, tmp, out+i);
			out[i] += w2*last;
			multiplicationAdditionKernel(iterationSize-1, w2, tmp, out+i+1, out+i+1);
			last = tmp[iterationSize-1];
			i += iterationSize;
		}
		iterationSize /= 2;
	}
	for(; i < count; i++) {
		float sample = in[i];
		float newLast = line.read(i1);
		out[i] = w1*newLast+w2*last;
		line.advance(sample);
		last = newLast;
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