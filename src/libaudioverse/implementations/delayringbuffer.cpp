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
	write_head = (write_head +1) &mask;
	buffer[write_head] = sample;
}

// For the below function.
const int ringbufferProcessingWorkspaceSize = 1024;
thread_local float ringbufferProcessingWorkspace[ringbufferProcessingWorkspaceSize];

// This is a very complicated function that took a lot of deep thought; touch carefully.
// There's a lot of subtlety here, and getting it wrong is very easy.
void DelayRingbuffer::process(unsigned int offset, int count, float* in, float* out) {
	// We can't read more than the distance between the offset and the write pointer without also advancing the write pointer.
	unsigned int maxIterationSize = offset < ringbufferProcessingWorkspaceSize ? offset : ringbufferProcessingWorkspaceSize;
	maxIterationSize = maxIterationSize < (unsigned int) count ? maxIterationSize : (unsigned int) count;
	// If we can't get enough to be worth copying, then we do this.
	if(maxIterationSize < 4) {
		for(int i = 0; i < count; i++) {
			float got = read(offset+1);
			advance(in[i]);
			out[i] = got;
		}
		return;
	}
	// Otherwise, we can grab entire sections at a time with std::copy, which should optimize to memcpy.
	while(count) {
		unsigned int read = (write_head-offset) & mask;
		// Determine availability for this iteration.
		// Our reads can and should include the position of write_head itself.
		// This is because offset 0 *is* the write head, so that delay lines with no delay can function.
		unsigned int copy = read < write_head ? write_head-read+1 : buffer_length-read;
		copy = copy < (unsigned int) count ? copy : (unsigned int) count;
		copy = copy < maxIterationSize ? copy : maxIterationSize;
		std::copy(buffer+read, buffer+read+copy, ringbufferProcessingWorkspace);
		// We have theoretically used the sample under write_head, so advance by 1.
		write_head = (write_head+1) & mask;
		// Advancing is done by moving write_head to the end of the buffer, then wrapping it and going again.
		unsigned int i = 0;
		while(i < copy) {
			unsigned int thisIteration = buffer_length-write_head;
			unsigned int remaining = copy-i;
			thisIteration = thisIteration < remaining ? thisIteration : remaining;
			std::copy(in+i, in+thisIteration+i, buffer+write_head);
			write_head = (write_head+thisIteration) & mask;
			i += thisIteration;
		}
		count -= copy;
		std::copy(ringbufferProcessingWorkspace, ringbufferProcessingWorkspace+copy, out);
		out += copy;
		in += copy;
		// write_head is sitting on the first unwritten sample, but needs to be on the first written sample.
		write_head = (write_head-1) & mask;
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