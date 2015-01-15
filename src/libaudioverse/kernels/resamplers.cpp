/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implements the high-quality, static resampler.*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/constants.hpp>
#include <algorithm>
#include <math.h>

void staticResamplerKernel(int inputSr, int outputSr, int inputLength, float* input, int* outputLengthDestination, float** outputDestination, bool makeMultipleOfFour) {
	if(inputSr == outputSr) { //just a copy.
		int neededLength = inputLength;
		if(neededLength%4 != 0 && makeMultipleOfFour) neededLength = 4*(neededLength/4+1);
		float* output = new float[neededLength]();
		std::copy(input, input+inputLength, output);
		*outputLengthDestination = neededLength;
		*outputDestination = output;
	}
	//we have to resample.
		float delta = (float)inputSr/outputSr;
	int pos = 0;
	float offset = 0;
	int neededLength = inputLength/delta;
	if(makeMultipleOfFour && neededLength%4 != 0) {
		neededLength = 4*(neededLength/4+1);
	}
	float* output = new float[neededLength]();
	int outputIndex = 0;
	while(pos < inputLength-1) {
		float w1 = 1-offset;
		float w2 = offset;
		int s1 = pos;
		int s2 = pos+1;
		output[outputIndex] = w1*input[s1]+w2*input[s2];
		outputIndex ++;
		offset += delta;	
		pos += (int)floorf(offset);
		offset -= floorf(offset);
	}
	*outputLengthDestination = neededLength;
	*outputDestination = output;
}
