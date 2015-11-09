/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

namespace libaudioverse_implementation {

/**DC blocking filter.

This filter does the best it can for any sample rate.

We have to use doubles internally in order to push the pole and zero close together without stability and quality issues.*/
class DcBlocker {
	public:
	DcBlocker(float _sr);
	float tick(float input);
	void reset();
	private:
	float sr;
	//Fixed 0, at DC, pole at almost-DC.
	double a1 = -0.9999;
	double h0 = 0.0;
};

inline DcBlocker::DcBlocker(float _sr): sr(_sr) {}

inline float DcBlocker::tick(float input) {
	//Direct form II, do the recursive filter first.
	double rec = input-a1*h0;
	double out = rec-h0;
	h0 = rec;
	return (float)out;
}

inline void DcBlocker::reset() {
	h0 = 0.0;
}

}