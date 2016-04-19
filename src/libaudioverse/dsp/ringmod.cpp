/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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