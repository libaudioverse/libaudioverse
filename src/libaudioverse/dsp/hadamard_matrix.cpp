/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <math.h>

namespace libaudioverse_implementation {

void hadamard(int n, float* buffer, bool shouldNormalize) {
	buffer[0] = 1.0f;
	if(n == 1) return; //because it's a matrix of order 1.
	for(int powerOfTwo = 2; powerOfTwo <= n; powerOfTwo *= 2) { //step through powers of 2.
		//Step through the smaller matrix we've already generated.
		int prevPowerOfTwo = powerOfTwo/2;
		for(int row = 0; row < prevPowerOfTwo; row++) {
			for(int column = 0; column < prevPowerOfTwo; column++) {
				//the two copies:
				buffer[row*n+column+prevPowerOfTwo] = buffer[row*n+column];
				buffer[(row+prevPowerOfTwo)*n+column] = buffer[row*n+column];
				//And the negation.
				buffer[(row+prevPowerOfTwo)*n+column+prevPowerOfTwo] = -buffer[row*n+column];
			}
		}
	}
	if(shouldNormalize) {
		float normFactor=1.0/sqrtf(n);
		scalarMultiplicationKernel(n*n, normFactor, buffer, buffer);
	}
}

}