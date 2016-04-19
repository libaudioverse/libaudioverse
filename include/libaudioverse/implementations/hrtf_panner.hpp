/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../private/hrtf.hpp"
#include "delayline.hpp"
#include "convolvers.hpp"
#include <memory>

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
	//Convolvers, current and previous.
	BlockConvolver *left_convolver, *right_convolver, *prev_left_convolver, *prev_right_convolver;
	int block_size;
	int response_length;
	float sr;
	float azimuth = 0, elevation = 0, prev_azimuth = 0, prev_elevation  = 0;
	float crossfade_threshold = 5.0;
	bool should_crossfade = true;
};

}