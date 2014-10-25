/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Knows how to take individual channels and combine them into an output buffer, or perform the inverse: separate a buffer of interleaved samples into individual buffers.*/
#include <libaudioverse/private_kernels.hpp>
#include <stdlib.h>
#include <string.h>

/**A brief note on the motivation:
We can allocate a chunk of memory which is the same size as the original buffer, but this will cause rapid heap fragmentation.
Alternatively, we can handle the samples in channels*channels matrices, i.e. channels frames of audio data at a time.
This is still somewhat bad because there is no global buffer reuse.
These are used seldomly in comparison with other kernels, and are unlikely to be a performance bottleneck.
Note: this is technically the transpose of a square matrix, but we have to use 1-dimensional arrays which makes that much less clear.
Todo: rewrrite this to use global buffers when such is implemented.
*/

void uninterleaveSamples(unsigned int channels, unsigned int frames, float* samples) {
	float* tempBlock = new float[channels*channels];

	//uninterleave them.
	//this loop steps by samples, not frames.
	//each iteration, i is at the next block's beginning.
	for(unsigned int i = 0; i < frames*channels; i+= channels*channels) {
		//fill the tempBlock with a transpose.
		for(unsigned int j = 0; j < channels; j++) {
			for(unsigned int k = 0; k < channels; k++) {
			tempBlock[j*channels+k] = i+k*channels+j > frames*channels ? 0 : samples[i+k*channels+j];
			}
		}
		//now, we splice it back in.
		for(unsigned int j = 0; j < channels*channels; j++) {
			if(i+j > frames*channels) break; //we hit the end of the input.
			samples[i+j] = tempBlock[j];
		}
	}
}

/**Since the transpose of a transpose is the original matrix, we actually have that uninterleaving uninterleaved samples interleaves them. Realy.
This function exists for readability.*/
void interleaveSamples(unsigned int channels, unsigned int frames, float* samples) {
	uninterleaveSamples(channels, frames, samples);
}
