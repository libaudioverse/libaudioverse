/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Knows how to take individual channels and combine them into an output buffer, or perform the converse: separate a buffer of interleaved samples into individual buffers.*/
#include <libaudioverse/private_interleaving.hpp>
#include <stdlib.h>
#include <string.h>

float** uninterleaveSamplesFast(unsigned int channels, unsigned int frames, float* samples) {
	float** out = (float**)calloc(channels, sizeof(float*));
	if(out == NULL) return NULL;
	for(unsigned int i = 0; i < channels; i++) {
		out[i] = (float*)calloc(frames, sizeof(float));
		if(out[i] == NULL) return NULL;
	}

	//uninterleave them.
	for(unsigned int offset = 0; offset < channels; offset ++) {
		for(unsigned int i = 0; i < frames; i++) out[offset][i] = samples[offset+i*channels];
	}
	return out;
}

float* interleaveSamplesFast(unsigned int channels, unsigned int count, float** uninterleavedSamples) {
	float* out = (float*)calloc(count*channels, sizeof(float));
	if(out == NULL) return NULL;
	for(unsigned int offset = 0; offset < channels; offset++) {
		for(unsigned int i = 0; i < count; i++) out[offset+channels*i] = uninterleavedSamples[offset][i];
	}
	return out;
}
