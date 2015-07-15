/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implements addition kernel.*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/memory.hpp>
#include <mmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

namespace libaudioverse_implementation {

float dotKernelSimple(int length, const float* v1, const float* v2) {
	float retval=0.0f;
	for(int i= 0; i < length; i++) retval += v1[i]*v2[i];
	return retval;
}

#if defined(LIBAUDIOVERSE_USE_SSE2)

float dotKernel(int length, const float* v1, const float* v2) {
	__m128 accum = _mm_setzero_ps();
	float result = 0.0f;
	for(int i= 0; i < length/4*4; i+=4) {
		accum = _mm_add_ps(accum, _mm_mul_ps(_mm_loadu_ps(v1+i), _mm_loadu_ps(v2+i)));
	}
	for(int i =length/4*4; i < length; i++) result += v1[i]*v2[i];
	//This next sequence gets accum[0] to be the sum of the floats in accum, so we can store it into result.
	//First, add the lower two to the upper two.
	//_mm_movehl_ps swaps the uppoer and lower halves, when called with the same argument twice.
	accum=_mm_add_ps(accum, _mm_movehl_ps(accum, accum));
	//Next, we want to add the lower two floats of accum.
	//_mm_shuffle_ps looks at the bits in an 8-bit integer.
	//When the argument is the same, The first two bits are where to read from for the lowest float, etc.
	//The result is to replace the lowest float in accum with the second-lowest float in accum.
	accum=_mm_add_ss(accum, _mm_shuffle_ps(accum, accum, 1));
	//extract.
	float tmp;
	_mm_store_ss(&tmp, accum);
	return result+tmp;
}

#else

float dotKernel(int length, const float* v1, const float* v2) {
	return dotKernelSimple(length, v1, v2);
}

#endif


}