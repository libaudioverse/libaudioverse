/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/workspace.hpp>
#include <libaudioverse/implementations/hrtf_panner.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/implementations/convolvers.hpp>
#include <algorithm>
#include <memory>
#include <math.h>

namespace libaudioverse_implementation {

thread_local Workspace<float> left_response_workspace, right_response_workspace;
thread_local Workspace<float> crossfade_workspace;

HrtfPanner::HrtfPanner(int _block_size, float _sr, std::shared_ptr<HrtfData> _hrtf): block_size(_block_size), sr(_sr), hrtf(_hrtf) {
	response_length = hrtf->getLength();
	float* left_response_ptr= left_response_workspace.get(response_length);
	float* right_response_ptr = right_response_workspace.get(response_length);
	hrtf->computeCoefficientsStereo(azimuth, elevation, left_response_ptr, right_response_ptr);
	left_convolver = new BlockConvolver(block_size);
	right_convolver = new BlockConvolver(block_size);
	prev_left_convolver = new BlockConvolver(block_size);
	prev_right_convolver = new BlockConvolver(block_size);
	left_convolver->setResponse(response_length, left_response_ptr);
	right_convolver->setResponse(response_length, right_response_ptr);
}

HrtfPanner::~HrtfPanner() {
	delete left_convolver;
	delete right_convolver;
	delete prev_left_convolver;
	delete prev_right_convolver;
}

void HrtfPanner::pan(float* input, float *left_output, float *right_output) {
	//Do we need to crossfade? We do if we've moved more than the threshold and crossfading is being allowed.
	bool needsCrossfade = should_crossfade && fabs(azimuth-prev_azimuth)+fabs(elevation-prev_elevation) >= crossfade_threshold;
	if(azimuth != prev_azimuth || elevation != prev_elevation) {
		if(needsCrossfade) {
			std::swap(left_convolver, prev_left_convolver);
			std::swap(right_convolver, prev_right_convolver);
			left_convolver->reset();
			right_convolver->reset();
		}
		float* left_response_ptr = left_response_workspace.get(response_length);
		float* right_response_ptr = right_response_workspace.get(response_length);
		hrtf->computeCoefficientsStereo(elevation, azimuth, left_response_ptr, right_response_ptr);
		left_convolver->setResponse(response_length, left_response_ptr);
		right_convolver->setResponse(response_length, right_response_ptr);
	}
	//These two convolutions always happen.
	left_convolver->convolve(input, left_output);
	right_convolver->convolve(input, right_output);
	if(needsCrossfade) {
		double delta = 1.0/block_size;
		float* crossfade_ptr = crossfade_workspace.get(block_size);
		prev_left_convolver->convolve(input, crossfade_ptr);
		for(int i = 0; i < block_size; i++) left_output[i] = (block_size-i)*delta*crossfade_ptr[i]+i*delta*left_output[i];
		prev_right_convolver->convolve(input, crossfade_ptr);
		for(int i = 0; i < block_size; i++) right_output[i] = (block_size-i)*delta*crossfade_ptr[i]+i*delta*right_output[i];
	}
	prev_azimuth = azimuth;
	prev_elevation = elevation;
}	

void HrtfPanner::reset() {
	left_convolver->reset();
	right_convolver->reset();
	prev_left_convolver->reset();
	prev_right_convolver->reset();
}

void HrtfPanner::setAzimuth(float angle) {
	azimuth = angle;
}

float HrtfPanner::getAzimuth() {
	return azimuth;
}

void HrtfPanner::setElevation(float angle) {
	elevation = angle;
}

float HrtfPanner::getElevation() {
	return elevation;
}

void HrtfPanner::setShouldCrossfade(bool cf) {
	should_crossfade = cf;
}

bool HrtfPanner::getShouldCrossfade() {
	return should_crossfade;
}

void HrtfPanner::setCrossfadeThreshold(float threshold) {
	crossfade_threshold = threshold;
}

float HrtfPanner::getCrossfadeThreshold() {
	return crossfade_threshold;
}


}