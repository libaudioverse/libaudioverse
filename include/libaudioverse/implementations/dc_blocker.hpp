/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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