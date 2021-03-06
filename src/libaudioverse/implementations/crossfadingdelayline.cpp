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

CrossfadingDelayLine::CrossfadingDelayLine(float maxDelay, float sr): line((int)(sr*maxDelay)+1) {
	this->sr = sr;
	max_delay = line.getLength();
}

void CrossfadingDelayLine::setDelay(float delay) {
	int newDelay = (unsigned int)(delay*sr);
	//Call to the slave is handled by setDelayInSamples.
	setDelayInSamples(newDelay);
}

void CrossfadingDelayLine::setDelayInSamples(int newDelay) {
	//we change newDelay, so call the slave first.
	if(slave) slave->setDelayInSamples(newDelay);
	if(newDelay >= max_delay) newDelay = max_delay-1;
	delay = new_delay;
	new_delay = newDelay;
	weight1 = 1.0f;
	weight2 = 0.0f;
	if(interpolation_time > 0.0) interpolation_delta = 1.0/(interpolation_time*sr);
	else interpolation_delta=1.0f;
	counter= (int)(1.0f/interpolation_delta);
	if(counter<= 0) counter=1;
}

void CrossfadingDelayLine::setInterpolationTime(float t) {
	interpolation_time = t;
	if(slave) slave->setInterpolationTime(t);
}

float CrossfadingDelayLine::tick(float sample) {
	float retval = computeSample();
	advance(sample);
	return retval;
}

void CrossfadingDelayLine::processBuffer(int length, float* input, float* output) {
	int cf = std::min(counter, length);
	int remaining =length-cf;
	float sample;
	for(int i = 0; i < cf; i++) {
		sample=input[i]; //save in case of in-place.
		output[i] = weight1*line.read(delay)+weight2*line.read(new_delay);
		line.advance(sample);
		counter--;
		weight1-=interpolation_delta;
		weight2+=interpolation_delta;
	}
	if(counter == 0 && cf > 0) { //we crossfaded, but finished this block.
		weight1 =1.0f;
		weight2=0.0f;
		delay = new_delay;
	}
	//We might have a bit more.
	for(int i = cf; i < length; i++) {
		sample= input[i];
		output[i] = line.read(delay);
		line.advance(sample);
	}
}

float CrossfadingDelayLine::computeSample() {
	if(counter) return weight1*line.read(delay)+weight2*line.read(new_delay);
	return line.read(delay);
}

void CrossfadingDelayLine::advance(float sample) {
	line.advance(sample);
	if(counter) {
		weight1 -= interpolation_delta;
		weight2 += interpolation_delta;
		counter--;
		if(counter== 0) {
			delay=new_delay;
			weight1 = 1.0f;
			weight2= 0.0f;
		}
	}
}

void CrossfadingDelayLine::write(float delay, float value) {
	int index = (int)(delay*sr);
	line.write(index, value);
}

void CrossfadingDelayLine::add(float delay, float value) {
	int index = (int)(delay*sr);
	line.write(index, value);
}

void CrossfadingDelayLine::reset() {
	weight1 = 1.0f;
	weight2 = 0.0f;
	counter=0;
	line.reset();
	if(slave) slave->reset();
}

CrossfadingDelayLine* CrossfadingDelayLine::getSlave() {
	return slave;
}

void CrossfadingDelayLine::setSlave(CrossfadingDelayLine* s) {
	slave = s;
}

}