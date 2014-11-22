/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_kernels.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/implementations/panner.hpp>
#include <libaudioverse/private_constants.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

void LavPannerImplementation::reset() {
	channels.clear();
}

void LavPannerImplementation::addEntry(float angle, unsigned int channel) {
	//note that clockwise/counterclockwise literally doesn't matter with this algorithm.
	//it is important only that we be consistent.
	float x = cos(angle);
	float y = sin(angle);
	channels.emplace_back(x, y, channel);
}

void LavPannerImplementation::pan(float angle, unsigned int block_size, float* input, float** outputs) {
	//the two degenerates: 0 and 1 channels.
	if(input == nullptr || outputs == nullptr) return;
	if(channels.size() == 0 || channels.size() == 1) {
		std::copy(input, input+block_size, outputs[channels[0].channel]);
		return;
	}
	float x = cos(angle);
	float y = sin(angle);
	//we use a lambda because we need to capture the above variables.
	std::function<bool(LavPannerEntry, LavPannerEntry)> comparer = [=](const LavPannerEntry &a, const LavPannerEntry &b) {
		float dist1 = (a.x-x)*(a.x-x)+(a.y-y)*(a.y-y);
		float dist2 = (b.x-x)*(b.x-x)+(b.y-y)*(b.y-y);
		return dist1 < dist2;
	};
	std::sort(channels.begin(), channels.end(), comparer);
	LavPannerEntry &closest = channels[0], &second_closest = channels[1];
	//dot product and acos give us angles. Magnitude is normalized to 1 already, no need for division.
	float dot1 = closest.x*x+closest.y*y;
	float dot2 = second_closest.x*x+second_closest.y*y;
	float angle1 = acos(dot1 > 1.0f ? 1.0f: dot1); //floating point error might trigger range issues.
	float angle2 = acos(dot2 > 1.0f ? 1.0f : dot2);
	float angleSum = angle1+angle2;
	float weight1 = angle1/angleSum;
	float weight2 = angle2/angleSum;
	unsigned int channel1 = closest.channel, channel2 = second_closest.channel;
	for(unsigned int i = 0; i < block_size; i++) {
		outputs[channel1][i] = weight1*input[i];
		outputs[channel2][i] = weight2*input[i];
	}
}
