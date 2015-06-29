/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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