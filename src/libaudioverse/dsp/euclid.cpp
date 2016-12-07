/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/private/dspmath.hpp>
#include <math.h>
#include <algorithm>

namespace libaudioverse_implementation {

//assumes a>=b.
int euclid(int a, int b) {
	while(b != 0) {
		int t=b;
		b=a%b;
		a=t;
	}
	return a;
}

int greatestCommonDivisor(int a, int b) {
	if(a < b) std::swap(a, b);
	return euclid(a, b);
}

}