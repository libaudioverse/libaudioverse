/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once

namespace libaudioverse_implementation {

/**InInterleaving and uninterleaving of samples.

Should the output count be less than channels, uninterleaving will only use the first outputCount channels.
Should the input count be less than channels, interleaving will assume zero for all remaining channels.
These two cases are rare and mainly exist to enable code reuse for getting blocks out of the simulation itself.*/
void uninterleaveSamples(unsigned int channels, unsigned int frames, float* samples, unsigned int outputCount, float** outputs);
void interleaveSamples(unsigned int channels, unsigned int frames, unsigned int inputCount, float** inputs, float* output);

//primitive math operations.
//It is safe to use these such that dest==a1 or dest==a2.
void additionKernel(int length, float* a1, float* a2, float* dest);
void scalarAdditionKernel(int length, float c, float*a1, float* dest);
void scalarMultiplicationKernel(int length, float c, float* a1, float* dest);
void multiplicationKernel(int length, float* a1, float* a2, float* dest);

//multiply a1 by c, sum with a2, and store result in dest.
//a1==dest and a2==dest are, again, safe.
void multiplicationAdditionKernel(int length, float c, float* a1, float* a2, float* dest);
//A parallel version, if we can, primarily used by convolution.
//This is equivalent to calling multiplicationAdditionKernel 4 times, advancing the  a1 pointer by 1 each time.
//This implies that a1 must be at least 3 elements longer than a2.
//Note that if a1 and a2 are the same buffers, this will be problematic; if they are, a2-a1 must be greater than 3.
void parallelMultiplicationAdditionKernel(int length, float c1, float c2, float c3, float c4,  float* a1, float* a2, float* out);

/**The convolution kernel.
The first response-1 samples of the input buffer are assumed to be a running history, so the actual length of the input buffer needs to be outputSampleCount+responseLength-1.
*/
void convolutionKernel(float* input, int outputSampleCount, float* output, int responseLength, float* response);

/**Same as convolutionKernel, but will crossfade from the first response to the second smoothly over the interval outputSampleCount.*/
void crossfadeConvolutionKernel(float* input, unsigned int outputSampleCount, float* output, unsigned int responseLength, float* from, float* to);

/**The panning kernels.  All arguments are self-explanatory.
The channel order must be the angles of all channels, specified clockwise where 0 is "in front" of the listener and angles proceed clockwise (this is to match HRTF; note that it is not standard trig).
Angles are in degrees, as this matches the hrtf algorithms and provides a better experience for exploring, etc.
The channel indices specify which angle goes with which channel; channelAngles[0] will write to channelIndices[0], etc.
Note: this writes to exactly and only 2 outputs.  Zero the others first.
*/

void amplitudePanKernel(float azimuth, float elevation, unsigned int inputLength, float* input, unsigned int numChannels, float** outputs, float* channelAngles, int* channelIndices);

/**Resamples a block of audio data.
This is slow and inefficient.  Frequent calls will cause heap fragmentation.  This is as high quality as possible. Intended use is one-off data that must be converted.

Note that this allocates with new[] because it just forwards onto speex_resampler_cpp.*/
void staticResamplerKernel(int inputSr, int outputSr, int channels, int frames, float* data, int *framesOut, float** dataOut);

/**Dot two vectors.*/
float dotKernel(int length, const float* v1, const float* v2);
}