/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

LavDelayRingbuffer::LavDelayRingbuffer(int length) {
	buffer_length = length;
	buffer = new float[length]();
}

LavDelayRingbuffer::~LavDelayRingbuffer() {
	delete[] buffer;
}

float LavDelayRingbuffer::read(int offset) {
	return buffer[ringmodi(write_head-offset, buffer_length)];
}

int LavDelayRingbuffer::getLength() {
	return buffer_length;
}

void LavDelayRingbuffer::advance(float sample) {
	write_head++;
	buffer[ringmodi(write_head, buffer_length)] = sample;
}

void LavDelayRingbuffer::write(int index, float value) {
	buffer[ringmodi(write_head-index, buffer_length)] = value;
}

void LavDelayRingbuffer::add(int index, float value) {
	buffer[ringmodi(write_head-index, buffer_length)] += value;
}

void LavDelayRingbuffer::reset() {
	memset(buffer, 0, sizeof(float)*buffer_length);
}
