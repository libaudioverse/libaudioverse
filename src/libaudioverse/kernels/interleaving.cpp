/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/

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