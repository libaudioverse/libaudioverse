/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <string>
#include <functional>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>
#include "libaudioverse.h"

/**A resampler.

This is a class because it, unlike the kernels, needs specific initialization.
It is the last piece in the pipeline, responsible for moving the Libaudioverse simulation sampling rate to the device's sampling rate.*/

/**Input buffer size must be even.*/
class LavResampler {
	public:
	LavResampler(int inputBufferLength, int inputSr, int outputSr);
	int getOutputBufferLength();
	void tick(float* input, float* output);
	private:
	float computeSingleSample(float* input);
	float last_sample = 0;
	float current_offset = 0;
	int current_pos = -1;//special sentinal value.
	bool no_op = false;
	float delta = 0.0f;
	int input_buffer_length, input_sr, output_sr;
};