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

LavResampler::LavResampler(int inputFrameCount, int inputChannels, int inputSr, int outputSr): input_frame_count(inputFrameCount), input_channels(inputChannels), input_sr(inputSr), output_sr(outputSr) {
	printf("Creating resampler.\n");
	printf("isr = %i, osr = %i\n", inputSr, outputSr);
	printf("Channels: %i\n", inputChannels);
	printf("Input frame count: %i\n", inputFrameCount);
	if(inputSr == outputSr) {
		no_op = true;
	}
	delta = (float)inputSr/(float)outputSr;
	last_frame = new float[inputChannels]();
	frame1 = new float[inputChannels];
	frame2 = new float[inputChannels];
	printf("No-op status: %i\n", no_op);
	printf("Predicted output size: %i", (int)floorf(input_frame_count/delta));
}

int LavResampler::getOutputFrameCount() {
	return (int)floorf(input_frame_count/delta);
}

void LavResampler::writeFrame(float* input, float* dest) {
	//weights are easy.
	float w1 = 1-current_offset;
	float w2 = current_offset;
	if(current_pos == -1) { //special case.
		std::copy(last_frame, last_frame+input_channels, frame1);
		std::copy(input, input+input_channels, frame2);
	}
	else {
		std::copy(input+current_pos*input_channels, input+(current_pos+1)*input_channels, frame2);
		std::copy(input+(current_pos+1)*input_channels, input+(current_pos+2)*input_channels, frame2);
	}
	for(int i = 0; i < input_channels; i++) {
		dest[i] = w1*frame1[i]+w2*frame2[i];
	}
}

void LavResampler::tick(float* input, float* output) {
	if(no_op) {
		std::copy(input, input+input_frame_count*input_channels, output);
		return;
	}
	//have to get this loop right.
	//for loop is a bit complicated and unclear. This while loop alternative is probably, for once, better.
	while(current_pos < input_frame_count -1) {
		writeFrame(input, output+(current_pos*input_channels));
		current_offset+=delta;
		current_pos += (int)floorf(current_offset);	
		current_offset = current_offset-floorf(current_offset);
	}
	current_pos = -1;
	std::copy(input+(input_frame_count-1)*input_channels, input+input_frame_count*input_channels, last_frame);
}
