#include "libaudioverse.h"
Lav_PUBLIC_FUNCTION float** uninterleaveSamplesFast(unsigned int channels, unsigned int frames, float* samples);
Lav_PUBLIC_FUNCTION float* interleaveSamplesFast(unsigned int channels, unsigned int count, float** uninterleavedSamples);
