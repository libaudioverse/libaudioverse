/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/


/**The convolution kernel.
The first response-1 samples of the input buffer are assumed to be a running history, so the actual length of the input buffer needs to be outputSampleCount+responseLength-1.
*/
void convolutionKernel(float* input, unsigned int outputSampleCount, float* output, unsigned int responseLength, float* response);

/**Same as convolutionKernel, but will crossfade from the first response to the second smoothly over the interval outputSampleCount.*/
void crossfadeConvolutionKernel(float* input, unsigned int outputSampleCount, float* output, unsigned int responseLength, float* from, float* to);

/**The panning kernels.  All arguments are self-explanatory.
The channel order must be the angles of all channels, specified clockwise where 0 is "in front" of the listener and angles proceed clockwise (this is to match HRTF; note that it is not standard trig).
Angles are in radians.
*/

void amplitudePanningKenrel(float azimuth, 