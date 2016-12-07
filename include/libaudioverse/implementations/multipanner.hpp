/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "hrtf_panner.hpp"
#include "amplitude_panner.hpp"
#include <memory>

namespace libaudioverse_implementation {

class HrtfData;

//This implementation has a cpp file because it does not provide a tick method.
//Also, putting it in the CPP file lets us avoid including libaudioverse_properties.h in a bunch of places we otherwise wouldn't.
class Multipanner {
	public:
	Multipanner(int _block_size, float _sr, std::shared_ptr<HrtfData> _hrtf_data);
	void process(float* input, float** outputs);
	float getAzimuth();
	void setAzimuth(float a);
	float getElevation();
	void setElevation(float e);
	bool getShouldCrossfade();
	void setShouldCrossfade(bool cf);
	//Manhattan distance (delta_azimuth+delta_elevation) before crossfading happens.
	float getCrossfadeThreshold();
	void setCrossfadeThreshold(float t);
	int getStrategy();
	void setStrategy(int s);
	private:
	HrtfPanner hrtf_panner;
	AmplitudePanner amplitude_panner;
	int block_size;
	float sr;
	int channels;
	int strategy;
	float azimuth = 0.0f, elevation = 0.0f;
	bool should_crossfade = true;
	float crossfade_threshold = 5.0f;
	std::shared_ptr<HrtfData> hrtf_data;
};

}