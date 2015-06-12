/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

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