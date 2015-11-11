/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/hrtf_panner.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/implementations/convolvers.hpp>
#include <kiss_fftr.h>
#include <algorithm>
#include <memory>
#include <math.h>

namespace libaudioverse_implementation {

HrtfPanner::HrtfPanner(int _block_size, float _sr, std::shared_ptr<HrtfData> _hrtf): block_size(_block_size), sr(_sr), hrtf(_hrtf) {
	response_length = hrtf->getLength();
	left_response = allocArray<float>(response_length);
	right_response = allocArray<float>(response_length);
	hrtf->computeCoefficientsStereo(azimuth, elevation, left_response, right_response);
	left_convolver = new FftConvolver(block_size);
	right_convolver = new FftConvolver(block_size);
	prev_left_convolver = new FftConvolver(block_size);
	prev_right_convolver = new FftConvolver(block_size);
	left_convolver->setResponse(response_length, left_response);
	right_convolver->setResponse(response_length, right_response);
	crossfade_workspace = allocArray<float>(block_size);
	input_fft = allocArray<kiss_fft_cpx>(left_convolver->getFftSize());
	fft_workspace = allocArray<float>(left_convolver->getFftSize());
	fft = kiss_fftr_alloc(left_convolver->getFftSize(), 0, nullptr, nullptr);
}

HrtfPanner::~HrtfPanner() {
	freeArray(left_response);
	freeArray(right_response);
	freeArray(crossfade_workspace);
	freeArray(input_fft);
	freeArray(fft_workspace);
	delete left_convolver;
	delete right_convolver;
	delete prev_left_convolver;
	delete prev_right_convolver;
	kiss_fft_free(fft);
}

void HrtfPanner::pan(float* input, float *left_output, float *right_output) {
	//Do we need to crossfade? We do if we've moved more than the threshold and crossfading is being allowed.
	bool needsCrossfade = should_crossfade && fabs(azimuth-prev_azimuth)+fabs(elevation-prev_elevation) >= crossfade_threshold;
	if(azimuth != prev_azimuth || elevation != prev_elevation) {
		std::swap(left_convolver, prev_left_convolver);
		std::swap(right_convolver, prev_right_convolver);
		left_convolver->reset();
		right_convolver->reset();
		hrtf->computeCoefficientsStereo(elevation, azimuth, left_response, right_response);
		left_convolver->setResponse(response_length, left_response);
		right_convolver->setResponse(response_length, right_response);
	}
	//FFT of the input.
	std::copy(input, input+block_size, fft_workspace);
	kiss_fftr(fft, fft_workspace, input_fft);
	//These two convolutions always happen.
	left_convolver->convolveFft(input_fft, left_output);
	right_convolver->convolveFft(input_fft, right_output);
	if(needsCrossfade) {
		double delta = 1.0/block_size;
		prev_left_convolver->convolveFft(input_fft, crossfade_workspace);
		for(int i = 0; i < block_size; i++) left_output[i] = (block_size-i)*delta*crossfade_workspace[i]+i*delta*left_output[i];
		prev_right_convolver->convolveFft(input_fft, crossfade_workspace);
		for(int i = 0; i < block_size; i++) right_output[i] = (block_size-i)*delta*crossfade_workspace[i]+i*delta*right_output[i];
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