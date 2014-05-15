#include "libaudioverse.h"
Lav_PUBLIC_FUNCTION float** uninterleaveSamplesFast(unsigned int frames, unsigned int channels, float* samples);
Lav_PUBLIC_FUNCTION float* interleaveSamplesFast(unsigned int count, unsigned int channels, float** uninterleavedSamples);
