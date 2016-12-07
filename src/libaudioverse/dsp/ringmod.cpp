/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/private/dspmath.hpp>
#include <math.h>

namespace libaudioverse_implementation {

//Apparently, C's mod is in fact not the discrete math operation: modulus of negative numbers does not follow the description needed for a ringbuffer.
//These function handles that.
int ringmodi(int dividend, int divisor) {
	const int result  = dividend%divisor;
	return result < 0 ? result + divisor : result;
}

float ringmodf(float dividend, float divisor) {
	const float result = fmodf(dividend, divisor);
	return result < 0 ? result+divisor : result;
}

double ringmod(double dividend, double divisor) {
	const double result  = fmod(dividend, divisor);
	return result < 0 ? result+divisor : result;
}

}