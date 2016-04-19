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