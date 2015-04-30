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
#include "../speex_resampler.h"

namespace libaudioverse_implementation {

/**A resampler.

This is a class because it, unlike the kernels, needs specific initialization.
It is the last piece in the pipeline, responsible for moving the Libaudioverse simulation sampling rate to the device's sampling rate.*/
class Resampler {
	public:
	Resampler(int inputFrameCount, int inputChannels, int inputSr, int outputSr);
	//returns frames written, not samples.
	int write(float* dest, int maxFrameCount);
	//this copies, the buffer can be reused.
	void read(float* source);
	//note the estimate: this is not necessarily sample-accurate. It's a rough estimate, primarily for the push node.
	int estimateAvailableFrames();
	private:
	float delta = 0.0f;
	std::list<float*> queue, done_queue;
	int offset=0, input_frame_count, input_channels, input_sr, output_sr;
	SpeexResamplerState* spx_resampler= nullptr;
	int spx_error= 0;
};

}