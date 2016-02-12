/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/amplitude_panner.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

AmplitudePanner::AmplitudePanner(int _block_size, float _sr):
sr(_sr), block_size(_block_size) {
}

void AmplitudePanner::clearMap() {
	channels.clear();
}

void AmplitudePanner::addEntry(float angle, int channel) {
	channels.emplace_back(ringmodf(angle, 360.0f), channel);
	std::sort(channels.begin(), channels.end(),
	[](AmplitudePannerEntry &a, AmplitudePannerEntry& b) {return a.angle < b.angle;});
}

void AmplitudePanner::pan(float* input, float** outputs) {
	//TODO: move all this logic into something that's only called when needed.
	//the two degenerates: 0 and 1 channels.
	if(input == nullptr || outputs == nullptr || channels.size() == 0) return;
	if(channels.size() == 1) {
		std::copy(input, input+block_size, outputs[channels[0].channel]);
		return;
	}
	//We need a local copy.
	float angle = azimuth;
	angle = ringmodf(angle, 360.0f);
	int left = 0, right = 0;
	bool found_right= false;
	for(int i = 0; i < channels.size(); i++) {
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
	int channel1 = channels[left].channel;
	int channel2 = channels[right].channel;
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
	scalarMultiplicationKernel(block_size, weight1, input, outputs[channel1]);
	scalarMultiplicationKernel(block_size, weight2, input, outputs[channel2]);
}

void AmplitudePanner::readMap(int entries, float* map) {
	clearMap();
	for(int i = 0; i < entries; i++) if(map[i] != INFINITY) addEntry(map[i], i);
}

float AmplitudePanner::getAzimuth() {
	return azimuth;
}

void AmplitudePanner::setAzimuth(float a) {
	azimuth = a;
}

float AmplitudePanner::getElevation() {
	return elevation;
}

void AmplitudePanner::setElevation(float e) {
	elevation = e;
}

}