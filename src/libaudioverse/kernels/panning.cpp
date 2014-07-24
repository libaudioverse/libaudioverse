/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implements some panning kernels.  This does not include HRTF.*/
#include <libaudioverse/private_kernels.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/private_constants.hpp>
#include <algorithm>

/**This is amplitude panning.

The degenerate case is one speaker; in this case, just memcpy.

For all other cases, find the two speakers which azimuth lies between and use them.
*/

void amplitudePanKernel(float azimuth, float elevation, unsigned int inputLength, float* input, unsigned int numChannels, float** outputs, float* channelAngles, int* channelIndices) {
	if(numChannels == 1) {
		std::copy(input, input+inputLength, outputs[0]);
		return;
	}

	//compute a1i and a2i, the indices of the two angles of interest.
	int a1i = 0, a2i = 0;
	float midAngle = ringmodf(azimuth, 360.0f);
	for(unsigned int i = 0; i < numChannels; i++) {
		a1i = i-1;
		if(channelAngles[i] > midAngle) break;
	}
	a2i = a1i+1;

	float leftAngle =  0, rightAngle = 0;
	//the angle is between the last and first speaker.
	//we need to rephrase this case to be that the angle is between the first and last speaker.
	if(midAngle > channelAngles[numChannels-1]) {
		a1i = numChannels-1;
		a2i = 0;
		midAngle = -(360.0f-midAngle);
		leftAngle = -(360.0f-channelAngles[a1i]);
		rightAngle = channelAngles[a2i];
	}
	//the angle is between the first and last speaker.
	//in this case, we need to bring a1i to the last speaker, and phrase leftAngle as negative.
	//the difference is that midAngle is already okay.
	else if(midAngle < channelAngles[0]) {
		a1i = numChannels-1;
		leftAngle = channelAngles[a1i];
		rightAngle = channelAngles[a2i];
		leftAngle = -(360.0f-leftAngle);
	}
	else {
		leftAngle = channelAngles[a1i];
		rightAngle = channelAngles[a2i];
	}

	//find a valuee between 0 and 1, representing the offset of the sound relative to the left speaker.
	float deltaAngle = rightAngle-leftAngle;
	float delta = (midAngle-leftAngle)/deltaAngle;
	//we might be in the slice in front of the listener.
	//if so, we have to do more stuff.
	if(leftAngle < 0) {
		//we can rephrase as distance from the midpoint.
		float halfDeltaAngle = deltaAngle/2.0f;
		delta = (halfDeltaAngle+midAngle)/deltaAngle;
	}

	//for now, just use a linear panning law.
	float leftAmplitude = 1-delta;
	float rightAmplitude = delta;
	int leftIndex = channelIndices[a1i];
	int rightIndex = channelIndices[a2i];

	//copy, then multiply.
	//no sense making more buffers.
	std::copy(input, input+inputLength, outputs[leftIndex]);
	std::copy(input, input+inputLength, outputs[rightIndex]);

	for(unsigned int i = 0; i < inputLength; i++) {
		outputs[leftIndex][i]*=leftAmplitude;
		outputs[rightIndex][i]*=rightAmplitude;
	}
}