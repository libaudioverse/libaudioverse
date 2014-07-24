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

class LavResampler {
	public:
	LavResampler(int sourceSr, int targetSr, std::function<std::tuple<int, float*>(void)> getFromCallback);
	float getNext();
	void read(int count, float* dest);
	private:
	int source_sr, target_sr;
	float delta = 0.0f;
	std::function<std::tuple<int, float*>(void)> get_from_callback;
	std::tuple<int, float*> current_pair = std::tuple<int, float*>(0, nullptr);
	int current_pos;
	float current_offset;
	float last_sample;
};
