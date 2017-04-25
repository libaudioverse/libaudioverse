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
#include <string.h>

namespace libaudioverse_implementation {

DelayRingbuffer::DelayRingbuffer(unsigned int length) {
	unsigned int neededLength = 1;
	while(neededLength <=length) neededLength<<= 1; //left shift until first power of two greater.
	buffer_length = neededLength;
	buffer = new float[buffer_length]();
	mask=buffer_length-1; //all bits set.
}

DelayRingbuffer::~DelayRingbuffer() {
	delete[] buffer;
}

float DelayRingbuffer::read(unsigned int offset) {
	return buffer[(write_head-offset) & mask];
}

unsigned int DelayRingbuffer::getLength() {
	return buffer_length;
}

void DelayRingbuffer::advance(float sample) {
	write_head++;
	buffer[write_head & mask] = sample;
}

void DelayRingbuffer::process(unsigned int offset, int count, float* in, float* out) {
	// We can't read more than the distance between the offset and the write pointer without also advancing the write pointer.
	unsigned int maxIterationSize = offset < 64 ? offset: 64;
	float tmp[64];
	// If we can't get enough to be worth copying, then we do this.
	if(maxIterationSize < 16) {
		for(int i = 0; i < count; i++) {
			float got = read(offset);
			advance(in[i]);
			out[i] = got;
		}
	}
	// Otherwise, we can grab entire sections at a time with std::copy, which should optimize to memcpy.
	while(count) {
		unsigned int read = (write_head-offset) & mask;
		// Determine availability for this iteration.
		unsigned int copy = read < write_head ? write_head-read : buffer_length-write_head;
		copy = copy < count ? copy : count;
		copy = copy < maxIterationSize ? copy : maxIterationSize;
		std::copy(buffer+read, buffer+read+copy, tmp);
		// Advancing is done by moving write_head to the end of the buffer, then wrapping it and going again.
		unsigned int advancing = copy;
		while(advancing) {
			unsigned int thisIteration = buffer_length-write_head;
			thisIteration = thisIteration < advancing ? thisIteration : advancing;
			std::copy(in, in+thisIteration, buffer+write_head);
			write_head = (write_head+thisIteration) & mask;
			advancing -= thisIteration;
		}
	}
}

void DelayRingbuffer::write(unsigned int offset, float value) {
	buffer[(write_head-offset) & mask] = value;
}

void DelayRingbuffer::add(unsigned int index, float value) {
	buffer[(write_head-index) & mask] += value;
}

void DelayRingbuffer::reset() {
	memset(buffer, 0, sizeof(float)*buffer_length);
}

}