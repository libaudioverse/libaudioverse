/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <string>
#include <functional>
#include <list>
#include <limits>
#include <algorithm>
#include <utility>
#include "libaudioverse.h"

/**A resampler.

This is a class because it, unlike the kernels, needs specific initialization.
It is the last piece in the pipeline, responsible for moving the Libaudioverse simulation sampling rate to the device's sampling rate.*/
class LavResampler {
	public:
	LavResampler(int inputFrameCount, int inputChannels, int inputSr, int outputSr);
	int write(float* dest, int maxFrameCount);
	//this copies, the buffer can be reused.
	void read(float* source);
	//note the estimate: this is not necessarily sample-accurate due to fp issues.
	int estimateAvailableFrames();
	private:
	void writeFrame(float* input, float* dest);
	float *last_frame = nullptr;
	float *frame1 = nullptr, *frame2 = nullptr;
	float current_offset = 0;
	int current_pos = -1;//special sentinal value.
	float delta = 0.0f;
	std::list<float*> queue, done_queue;
	int input_frame_count, input_channels, input_sr, output_sr;
};