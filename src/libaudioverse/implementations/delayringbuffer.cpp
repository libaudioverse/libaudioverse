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