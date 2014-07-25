/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_resampler.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/private_kernels.hpp>
#include <algorithm>
#include <utility>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <libaudioverse/private_dspmath.hpp>

LavResampler::LavResampler(int inputBufferLength, int inputChannels, int inputSr, int outputSr): input_buffer_length(inputBufferLength), input_channels(inputChannels), input_sr(inputSr), output_sr(outputSr) {
	if(inputSr == outputSr) {
		no_op = true;
	}
	delta = (float)inputSr/(float)outputSr;
}

int LavResampler::getOutputSampleCount() {
	return (int)floorf(delta*input_buffer_length);
}

void LavResampler::writeFrame(float* input, float* dest) {
}

void LavResampler::tick(float* input, float* output) {
	if(no_op) {
		std::copy(input, input+input_buffer_length*input_channels, output);
	}
	//have to get this loop right.
	//for loop is a bit complicated and unclear. This while loop alternative is probably, for once, better.

	current_pos = -1;
	last_sample = input[input_buffer_length-1];
}
