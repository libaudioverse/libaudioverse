/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

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