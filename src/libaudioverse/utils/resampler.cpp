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
	delta = (float)inputSr/(float)outputSr;
	last_frame = new float[inputChannels]();
	frame1 = new float[inputChannels];
	frame2 = new float[inputChannels];
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
		std::copy(input+current_pos*input_channels, input+(current_pos+1)*input_channels, frame1);
		std::copy(input+(current_pos+1)*input_channels, input+(current_pos+2)*input_channels, frame2);
	}
	for(int i = 0; i < input_channels; i++) {
		dest[i] = w1*frame1[i]+w2*frame2[i];
	}
}

void LavResampler::read(float* source) {
	float* buff;
	if(done_queue.empty()) {
		buff = new float[input_frame_count*input_channels];
	}
	else {
		buff = done_queue.front();
		done_queue.pop_front();
	}
	std::copy(source, source+input_channels*input_frame_count, buff);
	queue.push_back(buff);
}

int LavResampler::write(float* dest, int maxFrameCount) {
	if(queue.empty()) {
		return 0;
	}
	int count = 0;
	float* buff;
	while(count < maxFrameCount) {
		if(queue.empty()) break;
		buff = queue.front();
		while(current_pos < input_frame_count-1 && count < maxFrameCount) {
			writeFrame(buff, dest);
			dest += input_channels;
			count ++;
			current_offset += delta;
			current_pos += (int)floorf(current_offset);
			current_offset -= floorf(current_offset);
		}
		//this might be rollover.  If it is, we need to copy the last frame and put buff in done because we're about to replace buff.
		if(current_pos >= input_frame_count-1) {
			std::copy(buff+(input_frame_count-1)*input_channels, buff+input_frame_count*input_channels, last_frame);
			queue.pop_front();
			done_queue.push_front(buff);
			current_pos = -1;
		}
	}
	return count;
}

int LavResampler::estimateAvailableFrames() {
	float delta_rec = 1.0/delta;
	return queue.size()*delta_rec*input_frame_count;
}
