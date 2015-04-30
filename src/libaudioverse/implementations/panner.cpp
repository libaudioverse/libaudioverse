/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/panner.hpp>
#include <libaudioverse/private/constants.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

void PannerImplementation::reset() {
	channels.clear();
}

void PannerImplementation::addEntry(float angle, unsigned int channel) {
	channels.emplace_back(ringmodf(angle, 360.0f), channel);
	std::sort(channels.begin(), channels.end(),
	[](PannerEntry &a, PannerEntry& b) {return a.angle < b.angle;});
}

void PannerImplementation::pan(float angle, unsigned int block_size, float* input, unsigned int outputCount, float** outputs) {
	//the two degenerates: 0 and 1 channels.
	if(input == nullptr || outputs == nullptr) return;
	if(channels.size() == 0 || channels.size() == 1) {
		std::copy(input, input+block_size, outputs[channels[0].channel]);
		return;
	}
	angle = ringmodf(angle, 360.0f);
	unsigned int left = 0, right = 0;
	bool found_right= false;
	for(unsigned int i = 0; i < channels.size(); i++) {
		if(channels[i].angle >= angle) {
			right = i;
			found_right = true;
			break;
		}
	}
	if(found_right == false) {
		right = 0;
		left = channels.size()-1;
	}
	else {
		left = right == 0 ? channels.size()-1 : right-1;
	}
	unsigned int channel1 = channels[left].channel;
	unsigned int channel2 = channels[right].channel;
	//two cases: we wrapped or didn't.
	float angle1, angle2, angleSum;
	if(right == 0) { //left is all the way around, special handling is needed.
		if(angle >= channels[left].angle) {
			angle1 = angle-channels[left].angle; //angle between left and source.
			angle2 = (360.0f-angle)+channels[right].angle;
		}
		else {
			angle1 = (360-channels[left].angle)+angle;
			angle2 = fabs(channels[right].angle-angle);
		}
	}
	else {
		angle1 = fabs(channels[left].angle-angle);
		angle2 = fabs(channels[right].angle-angle);
	}
	angleSum = angle1+angle2;
	float weight2 = angle1/angleSum;
	float weight1 = angle2/angleSum;
	for(unsigned int i = 0; i < outputCount; i++) {
		if(i == channel1) for(unsigned int j = 0; j < block_size; j++) outputs[i][j] = weight1*input[j];
		else if(i == channel2) for(unsigned int j = 0; j < block_size; j++) outputs[i][j] = weight2*input[j];
		else memset(outputs[i], 0, sizeof(float)*block_size);
	}
}

}