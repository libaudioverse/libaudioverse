/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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
	float crossfade_threshold = 0.1;
	bool should_crossfade = true;
};

}