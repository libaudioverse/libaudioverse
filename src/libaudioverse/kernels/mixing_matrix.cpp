/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Knows how to take individual channels and combine them into an output buffer, or perform the inverse: separate a buffer of interleaved samples into individual buffers.*/
#include <libaudioverse/private/kernels.hpp>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

void applyMixingMatrix(unsigned int inSampleCount, float* inSamples, float* outSamples, unsigned int inChannels, unsigned int outChannels, float* matrix) {
	for(float *inFrame = inSamples, *outFrame = outSamples; inFrame < inSamples+inSampleCount; inFrame+=inChannels, outFrame+=outChannels) {
		for(unsigned int row = 0; row < outChannels; row++) {
			float accum = 0.0f;
			for(unsigned int column = 0; column < inChannels; column++) accum += matrix[row*inChannels+column]*inFrame[column];
			outFrame[row] = accum;
		}
	}
}
