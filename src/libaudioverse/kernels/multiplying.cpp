/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implements addition kernel.*/
#include <libaudioverse/private_kernels.hpp>
#include <mmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

void multiplicationKernelSimple(int length, float* a1, float* a2, float* dest) {
	for(int i = 0; i < length; i++) dest[i]=a1[i]*a2[i];
}

#if defined(LIBAUDIOVERSE_USE_SSE2)
void multiplicationKernel(int length, float* a1, float* a2, float* dest) {
	int neededLength = (length/4)*4;
	__m128 a1r, a2r;
	for(int i = 0; i < neededLength; i+= 4) {
		a1r = _mm_load_ps(a1+i);
		a2r = _mm_load_ps(a2+i);
		a2r=_mm_mul_ps(a1r, a2r);
		_mm_store_ps(dest+i, a2r);
	}
	multiplicationKernelSimple(length-neededLength, a1+neededLength, a2+neededLength, dest+neededLength);
}
#else
void multiplicationKernel(int length, float* a1, float* a2, float* dest) {
	multiplicationKernelSimple(length, a1, a2, dest);
}
#endif

