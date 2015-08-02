/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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
//found in multiplying.cpp
//multiply a1 by c, sum with a2, and store result in dest.
//a1==dest and a2==dest are, again, safe.
void multiplicationAdditionKernel(int length, float c, float* a1, float* a2, float* dest);


/**The convolution kernel.
The first response-1 samples of the input buffer are assumed to be a running history, so the actual length of the input buffer needs to be outputSampleCount+responseLength-1.
*/
void convolutionKernel(float* input, unsigned int outputSampleCount, float* output, unsigned int responseLength, float* response);

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