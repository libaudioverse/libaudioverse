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