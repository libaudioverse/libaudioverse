/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

/**Implements the convolution kernel.*/
#include <libaudioverse/private/kernels.hpp>
#include <string.h>
#include <libaudioverse/private/memory.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

void convolutionKernel(float* input, int outputSampleCount, float* output, int responseLength, float* response) {
	std::fill(output, output+outputSampleCount, 0.0f);
	int parCount = responseLength/4*4;
	for(int i = 0; i < parCount; i += 4) {
		parallelMultiplicationAdditionKernel(outputSampleCount, response[responseLength-i-1], response[responseLength-i-2], response[responseLength-i-3], response[responseLength-i-4],
		input, output, output);
		input+=4;
	}
	for(int i = parCount; i < responseLength; i++) {
		float c=response[responseLength-i-1];
		multiplicationAdditionKernel(outputSampleCount, c, input, output, output);
		input++;
	}
}

void crossfadeConvolutionKernel(float* input, unsigned int outputSampleCount, float* output, unsigned int responseLength, float* from, float* to) {
	float delta = 1.0f/outputSampleCount;
	for(unsigned int i = 0; i < outputSampleCount; i++) {
		float weight1 = 1.0f-delta*i;
		float weight2 = i*delta;
		float samp = 0.0f;
		for(unsigned int j = 0; j < responseLength; j++) {
			samp += input[i + j] * (weight1*from[responseLength-j-1] + weight2*to[responseLength-j-1]);
		}
	output[i] = samp;
	}
}

}