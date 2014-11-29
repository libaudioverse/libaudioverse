/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

//A single channel delay line.
//This rounds to the nearest sample for fractional delays.

class LavDelayLine {
	LavDelayLine(float maxDelay, float sr);
	~LavDelayLine();
	void setDelay(float delay);
	float read();
	void advance(float sample);
	void setInterpolationDelta(float d);
	private:
	float* line = nullptr;
	unsigned int line_length = 0, delay = 0, write_head = 0, new_delay = 0, new_write_head = 0;
	bool is_interpolating = false;
	float sr = 0, interpolation_delta = 1.0f;
	float weight1=1.0f, weight2=0.0f;
};
