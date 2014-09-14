/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

class LavBiquadFilter {
	public:
	LavBiquadFilter();
	void configure(int type, double sr, double frequency, double dbGain, double q);
	//advance the biquad by one sample.
	float tick(float sample);
	void clearHistories();
	private:
	double a1=0, a2=0, b1=0, b2=0, gain = 0;
	float history[2];
	double recursion_history[2];
};

