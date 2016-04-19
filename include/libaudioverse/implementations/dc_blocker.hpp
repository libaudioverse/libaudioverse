/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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