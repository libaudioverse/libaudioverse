/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

/**Implements multiplication kernel and vairiants.*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/memory.hpp>
#include <mmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

namespace libaudioverse_implementation {

void multiplicationKernelSimple(int length, float* a1, float* a2, float* dest) {
	for(int i = 0; i < length; i++) dest[i]=a1[i]*a2[i];
}

void scalarMultiplicationKernelSimple(int length, float c, float* a1, float* dest) {
	for(int i = 0; i < length; i++) dest[i]=c*a1[i];
}

#if defined(LIBAUDIOVERSE_USE_SSE2)
void multiplicationKernel(int length, float* a1, float* a2, float* dest) {
	int neededLength = (length/4)*4;
	__m128 a1r, a2r;
	for(int i = 0; i < neededLength; i+= 4) {
		a1r = _mm_loadu_ps(a1+i);
		a2r = _mm_loadu_ps(a2+i);
		a2r=_mm_mul_ps(a1r, a2r);
		_mm_storeu_ps(dest+i, a2r);
	}
	multiplicationKernelSimple(length-neededLength, a1+neededLength, a2+neededLength, dest+neededLength);
}

void scalarMultiplicationKernel(int length, float c, float* a1, float* dest) {
	int neededLength = (length/4)*4;
	__m128 a1r, cr;
	cr = _mm_load1_ps(&c);
	for(int i = 0; i < neededLength; i+= 4) {
		a1r = _mm_loadu_ps(a1+i);
		a1r=_mm_mul_ps(a1r, cr);
		_mm_storeu_ps(dest+i, a1r);
	}
	scalarMultiplicationKernelSimple(length-neededLength, c, a1+neededLength, dest+neededLength);
}

#else
void multiplicationKernel(int length, float* a1, float* a2, float* dest) {
	multiplicationKernelSimple(length, a1, a2, dest);
}

void scalarMultiplicationKernel(int length, float c, float* a1, float*dest) {
	scalarMultiplicationKernelSimple(length, c, a1, dest);
}

#endif

}