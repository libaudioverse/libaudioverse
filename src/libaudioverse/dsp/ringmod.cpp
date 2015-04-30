/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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