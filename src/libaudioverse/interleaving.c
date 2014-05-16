/**Knows how to take individual channels and combine them into an output buffer, or perform the converse: separate a buffer of interleaved samples into individual buffers.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>

Lav_PUBLIC_FUNCTION float** uninterleaveSamplesFast(unsigned int channels, unsigned int frames, float* samples) {
	float** out = calloc(channels, sizeof(float*));
	if(out == NULL) return NULL;
	for(unsigned int i = 0; i < channels; i++) {
		out[i] = calloc(frames, sizeof(float));
		if(out[i] == NULL) return NULL;
	}

	//uninterleave them.
	for(unsigned int offset = 0; offset < channels; offset ++) {
		for(unsigned int i = 0; i < frames; i++) out[offset][i] = samples[offset+i*channels];
	}
	return out;
}

Lav_PUBLIC_FUNCTION float* interweaveSamplesFast(unsigned int channels, unsigned int count, float** uninterleavedSamples) {
	float* out = calloc(count*channels, sizeof(float));
	if(out == NULL) return NULL;
	for(unsigned int offset = 0; offset < channels; offset++) {
		for(unsigned int i = 0; i < count; i++) out[offset+channels*i] = uninterleavedSamples[offset][i];
	}
	return out;
}
