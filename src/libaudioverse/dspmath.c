/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <math.h>

//Apparently, C's mod is in fact not the discrete math operation: modulus of negative numbers does not follow the description needed for a ringbuffer.
//These function handles that.
Lav_PUBLIC_FUNCTION int ringmodi(int dividend, int divisor) {
	return dividend < 0 ? dividend%divisor+divisor : dividend%divisor;
}

Lav_PUBLIC_FUNCTION float ringmodf(float dividend, float divisor) {
	return dividend < 0 ? fmodf(dividend, divisor)+divisor : fmodf(dividend, divisor);
}

Lav_PUBLIC_FUNCTION double ringmod(double dividend, double divisor) {
	return dividend < 0 ? fmod(dividend, divisor)+divisor : fmod(dividend, divisor);
}
