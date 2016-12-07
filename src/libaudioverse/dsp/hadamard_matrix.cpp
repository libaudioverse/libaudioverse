/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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