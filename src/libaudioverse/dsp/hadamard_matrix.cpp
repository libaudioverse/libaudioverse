/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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