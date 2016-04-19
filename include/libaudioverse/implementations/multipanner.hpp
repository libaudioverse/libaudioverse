/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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