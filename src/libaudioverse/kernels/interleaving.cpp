/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Knows how to take individual channels and combine them into an output buffer, or perform the inverse: separate a buffer of interleaved samples into individual buffers.*/
#include <libaudioverse/private_kernels.hpp>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

/*These functions are not written to be fast, as they are rarely used.
It is, however, very important that they avoid allocating memory if at all possible.
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
				tempBlock[j*channels+k] = i+k*channels+j >= frames*channels ? 0 : samples[i+k*channels+j];
			}
		}
		//now, we splice it back in.
		for(unsigned int j = 0; j < channels*channels; j++) {
			if(i+j >= frames*channels) break; //we hit the end of the input.
			samples[i+j] = tempBlock[j];
		}
	}
}

void interleaveSamples(unsigned int channels, unsigned int frames, float* samples) {
	//if channels is 1, drop out now and save ourselves a very big loop.
	if(channels == 1) return;
	//we want to interleave the first block in such a way that we end up with a following uninterleaved buffer.
	//base case: frames = 1 or 0, so return.
	while(true) {
		if(frames == 1 || frames == 0) return;
		/*Otherwise, we want the first channel to be uninterleaved.
Suppose we grab the first sample of the second channel, move the first channel right by one sample, and place it in the first position.
Suppose we grab the first sample of the third channel, move the second and third channels right by one sample, and place it in the second position...etc.
Then reverse the first frame of audio and recurse.*/
		float tempSample = 0.0f;
		for(unsigned int i = 1; i < channels; i++) {
			tempSample = samples[i*frames]; //2nd, 3rd, 4th, 5th, etc...
			//move everything to the left of where we grabbed tempSample to the right by 1.
			memmove(samples+1, samples, sizeof(float)*i*frames);
			samples[0] = tempSample; //put it in place.
		}
		//reverse the first frame.
		std::reverse(samples, samples+channels);
		frames-=1;
		samples+=channels;
	}
}