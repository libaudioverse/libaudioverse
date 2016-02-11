/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implements multiplication kernel and vairiants.*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/memory.hpp>
#include <mmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

namespace libaudioverse_implementation {

void multiplicationAdditionKernelSimple(int length, float c, float* a1, float *a2, float* dest) {
	for(int i = 0; i < length; i++) dest[i]=c*a1[i]+a2[i];
}


void parallelMultiplicationAdditionKernelSimple(int length, float c1, float c2, float c3, float c4, float* a1, float* a2, float* out) {
	float *a11 = a1, *a12 = a1+1, *a13 = a1+2, *a14 = a1+3;
	for(int i = 0; i < length; i++) {
		out[i] = a2[i]+a11[i]*c1+a12[i]*c2+a13[i]*c3+a14[i]*c4;
	}
}

#if defined(LIBAUDIOVERSE_USE_SSE2)

void multiplicationAdditionKernel(int length, float c, float* a1, float* a2, float* dest) {
	int neededLength = (length/4)*4;
	__m128 cr = _mm_load1_ps(&c);
	for(int i = 0; i < neededLength; i+=4) {
		__m128 a1r, a2r;
		a1r = _mm_loadu_ps(a1+i);
		a2r = _mm_loadu_ps(a2+i);
		a2r = _mm_add_ps(a2r, _mm_mul_ps(a1r, cr));
		_mm_storeu_ps(dest+i, a2r);
	}
	multiplicationAdditionKernelSimple(length-neededLength, c, a1+neededLength, a2+neededLength, dest+neededLength);
}

void parallelMultiplicationAdditionKernel(int length, float c1, float c2, float c3, float c4, float* a1, float* a2, float* out) {
	__m128 c1r = _mm_set1_ps(c1);
	__m128 c2r = _mm_set1_ps(c2);
	__m128 c3r = _mm_set1_ps(c3);
	__m128 c4r = _mm_set1_ps(c4);
	int needed = length/4*4;
	for(int i = 0; i < needed; i+=4) {
		__m128 a1r = _mm_loadu_ps(a1+i);
		__m128 a2r = _mm_loadu_ps(a1+i+1);
		__m128 a3r = _mm_loadu_ps(a1+i+2);
		__m128 a4r = _mm_loadu_ps(a1+i+3);
		a1r = _mm_mul_ps(a1r, c1r);
		a2r = _mm_mul_ps(a2r, c2r);
		a3r = _mm_mul_ps(a3r, c3r);
		a4r = _mm_mul_ps(a4r, c4r);
		//Reuse these to avoid having to move registers around and stuff, in theory.
		//In practice the compiler might optimize this, but possibly not always, and it's not hard to do it ourselves.
		a1r = _mm_add_ps(a1r, a2r);
		a3r = _mm_add_ps(a3r, a4r);
		a2r = _mm_add_ps(a1r, a3r);
		a4r = _mm_loadu_ps(a2+i);
		_mm_storeu_ps(out+i, _mm_add_ps(a2r, a4r));
	}
	parallelMultiplicationAdditionKernelSimple(length-needed, c1, c2, c3, c4, a1+needed, a2+needed, out+needed);
}

#else

void multiplicationAdditionKernel(int length, float c, float* a1, float* a2, float* dest) {
	multiplicationAdditionKernelSimple(length, c, a1, a2, dest);
}

void parallelMultiplicationAdditionKernel(int length, float c1, float c2, float c3, float c4, float* a1, float* a2, float* out) {
	parallelMultiplicationKernelSimple(length, c1, c2, c3, c4, a1, a2, out);
}

#endif

}