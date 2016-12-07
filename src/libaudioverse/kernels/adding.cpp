/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

/**Implements addition kernel.*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/memory.hpp>
#include <mmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

namespace libaudioverse_implementation {

void additionKernelSimple(int length, float* a1, float* a2, float* dest) {
	for(int i = 0; i < length; i++) dest[i]=a1[i]+a2[i];
}

void scalarAdditionKernelSimple(int length, float c, float* a1, float* dest) {
	for(int i=0; i < length; i++) dest[i]=c+a1[i];
}

#if defined(LIBAUDIOVERSE_USE_SSE2)

void additionKernel(int length, float* a1, float* a2, float* dest) {
	int neededLength = (length/4)*4;
	__m128 a1r, a2r;
	for(int i = 0; i < neededLength; i+= 4) {
		a1r = _mm_loadu_ps(a1+i);
		a2r = _mm_loadu_ps(a2+i);
		a2r=_mm_add_ps(a1r, a2r);
		_mm_storeu_ps(dest+i, a2r);
	}
	additionKernelSimple(length-neededLength, a1+neededLength, a2+neededLength, dest+neededLength);
}

void scalarAdditionKernel(int length, float c, float* a1, float* dest) {
	__m128 cr = _mm_load1_ps(&c);
	int blocks = length/4;
	for(int i = 0; i < blocks*4; i+=4) {
		__m128 r1=_mm_loadu_ps(a1+i);
		r1 = _mm_add_ps(r1, cr);
		_mm_storeu_ps(dest+i, r1);
	}
	scalarAdditionKernelSimple(length-blocks*4, c, a1, dest);
}

#else
void additionKernel(int length, float* a1, float* a2, float* dest) {
	additionKernelSimple(length, a1, a2, dest);
}

void scalarAdditionKernel(int length, float c, float* a1, float* dest) {
	scalarAdditionKernelSimple(length, c, a1, dest);
}

#endif


}