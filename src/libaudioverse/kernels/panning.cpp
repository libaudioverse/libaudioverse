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

void amplitudePanningKernel(float azimuth, float elevation, unsigned int inputLength, float* input, unsigned int numChannels, float** outputs, float* channelMap) {
	if(numChannels == 1) {
		std::copy(input, input+inputLength, outputs[0]);
		return;
	}

	//compute a1i and a2i, the indices of the two angles of interest.
	int a1i = 0, a2i = 0;
	for(unsigned int i = 0; i < numChannels; i++) {
		a1i = i-1;
		if(channelMap[i] > azimuth) break;
	}
	a2i = a1i+1;
	a1i = ringmodi(a1i, numChannels);
	a2i = ringmodi(a2i, numChannels);
	float midAngle = ringmodf(azimuth, 2*PI);
	float leftAngle = channelMap[a1i];
	float rightAngle = channelMap[a2i];
	///there is a point in front of the listener where leftAngle>midAngle>rightAngle.
	//this fixes that case, such that the following algebra works.
	if(leftAngle > midAngle) {
		leftAngle = 360.0f-leftAngle;
	}

	//find a valuee between 0 and 1, representing the offset of the sound relative to the left speaker.
	float deltaAngle = rightAngle-leftAngle;
	float delta = (midAngle-leftAngle)/deltaAngle;

	//for now, just use a linear panning law.
	float leftAmplitude = 1-delta;
	float rightAmplitude = delta;

	//copy, then multiply.
	//no sense making more buffers.
	std::copy(input, input+inputLength, outputs[a1i]);
	std::copy(input, input+inputLength, outputs[a2i]);

	for(unsigned int i = 0; i < inputLength; i++) {
		outputs[a1i][i]*=leftAmplitude;
		outputs[a2i][i]*=rightAmplitude;
	}
}