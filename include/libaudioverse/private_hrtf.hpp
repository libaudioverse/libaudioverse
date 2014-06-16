/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include <string>

class LavHrtfData {
	public:
	//get the appropriate coefficients for one channel.  A stereo hrtf is two calls to this function.
	void hrtfComputeCoefficientsMono(LavHrtfData *hrtf, float elevation, float azimuth, float* out);

	//warning: writes directly to the output destination, doesn't allocate a new one.
	void hrtfComputeCoefficients(LavHrtfData *hrtf, float elevation, float azimuth, float* left, float* right);

	//load from a file.
	loadFromFile(std::string path);
	private:
	unsigned int elev_count, hrir_count, hrir_length;
	int min_elevation, max_elevation;
	unsigned int *azimuth_counts;
	unsigned int samplerate;
	float ***hrirs;
};

