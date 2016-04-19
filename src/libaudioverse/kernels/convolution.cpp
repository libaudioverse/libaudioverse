/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/

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