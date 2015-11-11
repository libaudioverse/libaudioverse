/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/hrtf.hpp"
#include "delayline.hpp"
#include "convolvers.hpp"
#include <memory>
#include <kiss_fftr.h>

namespace libaudioverse_implementation {

//Implement HRTF panning.

class HrtfPanner {
	public:
	HrtfPanner(int _block_size, float _sr, std::shared_ptr<HrtfData> hrtf);
	~HrtfPanner();
	void pan(float* input, float* left_output, float* right_output);
	void reset();
	void setAzimuth(float angle);
	float getAzimuth();
	void setElevation(float angle);
	float getElevation();
	void setShouldCrossfade(bool cf);
	bool getShouldCrossfade();
	//The threshold after which we will crossfade as (azimuth+elevation) > threshold.
	//Also known as manhattan distance.
	void setCrossfadeThreshold(float threshold);
	float getCrossfadeThreshold();
	private:
	std::shared_ptr<HrtfData> hrtf;
	//Workspaces and pointers:
	//Left and right HRIR responses.
	float *left_response = nullptr, *right_response = nullptr;
	//Convolvers, current and previous.
	FftConvolver *left_convolver, *right_convolver, *prev_left_convolver, *prev_right_convolver;
	//When crossfading, we calculate both outputs and then fade.
	float* crossfade_workspace;
	//The fft of our input can be calculated once and reused for up to 4 convolutions with the FftConvolver.
	//We go to the size the convolver wants, so we need a temporary buffer as well.
	kiss_fftr_cfg fft;
	kiss_fft_cpx *input_fft;
	float *fft_workspace = nullptr;
	int block_size;
	int response_length;
	float sr;
	float azimuth = 0, elevation = 0, prev_azimuth = 0, prev_elevation  = 0;
	float crossfade_threshold = 0;
	bool should_crossfade = true;
};

}