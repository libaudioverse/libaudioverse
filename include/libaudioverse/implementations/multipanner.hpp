/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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