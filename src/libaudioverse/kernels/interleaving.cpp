/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

/**Knows how to take individual channels and combine them into an output buffer, or perform the inverse: separate a buffer of interleaved samples into individual buffers.*/
#include <libaudioverse/private/kernels.hpp>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace libaudioverse_implementation {

void uninterleaveSamples(unsigned int channels, unsigned int frames, float* samples, unsigned int outputCount, float** outputs) {
	for(unsigned int i = 0; i < channels; i++) {
		if(i >= outputCount) break;
		for(unsigned int j = 0; j < frames; j++) {
			outputs[i][j] = samples[j*channels+i];
		}
	}
}

void interleaveSamples(unsigned int channels, unsigned int frames, unsigned int inputCount, float** inputs, float* output) {
	for(unsigned int i = 0; i < channels; i++) {
		for(unsigned int j = 0; j < frames; j++) {
			output[j*channels+i] = i >= inputCount ? 0.0f : inputs[i][j];
		}
	}
}

}