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

void householder(int n, float* buffer, bool shouldNormalize) {
	//The product of two identical vectors m*(1, 1, 1...) is the n by n matrix whose entries are m^2.
	//To normalize, use 1/sqrt(n) as the components, which is 1/n for the entries of the matrix.
	//Formula for householder: i-2 v v^t
	//where i is identity, v is a column unit vector, v^t is the transpose of v.
	float subtracting=2.0f/n;
	for(int i =0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			buffer[i*n+j] = i==j ? 1.0f-subtracting: -subtracting;
		}
	}
	if(shouldNormalize) {
		//The entries in each row are the same.
		float magnitude=(1.0f-subtracting)*(1.0f-subtracting)+(n-1)*subtracting*subtracting;
		magnitude=sqrtf(magnitude);
		scalarMultiplicationKernel(n, 1.0f/magnitude, buffer, buffer);;
	}
}

}