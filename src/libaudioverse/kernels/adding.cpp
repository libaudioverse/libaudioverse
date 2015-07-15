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