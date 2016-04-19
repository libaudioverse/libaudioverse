/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/implementations/amplitude_panner.hpp>
#include <libaudioverse/implementations/hrtf_panner.hpp>
#include <libaudioverse/implementations/multipanner.hpp>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/data.hpp>
#include <memory>

namespace libaudioverse_implementation {

Multipanner::Multipanner(int _block_size, float _sr, std::shared_ptr<HrtfData> _hrtf_data):
block_size(_block_size), sr(_sr), channels(2), strategy(Lav_PANNING_STRATEGY_STEREO),
hrtf_data(_hrtf_data),
hrtf_panner(_block_size, _sr, _hrtf_data),
amplitude_panner(_block_size, _sr) {
}

void Multipanner::process(float* input, float** outputs) {
	if(strategy = Lav_PANNING_STRATEGY_HRTF) {
		hrtf_panner.setShouldCrossfade(should_crossfade);
		hrtf_panner.setCrossfadeThreshold(crossfade_threshold);
		hrtf_panner.setAzimuth(azimuth);
		hrtf_panner.setElevation(elevation);
		hrtf_panner.pan(input, outputs[0], outputs[1]);
	}
	else {
		amplitude_panner.setAzimuth(azimuth);
		amplitude_panner.setElevation(elevation);
		amplitude_panner.pan(input,  outputs);
	}
}

float Multipanner::getAzimuth() {
	return azimuth;
}

void Multipanner::setAzimuth(float a) {
	azimuth = a;
}

float Multipanner::getElevation() {
	return elevation;
}

void Multipanner::setElevation(float e) {
	elevation = e;
}

bool Multipanner::getShouldCrossfade() {
	return should_crossfade;
}

void Multipanner::setShouldCrossfade(bool cf) {
	should_crossfade = cf;
}

float Multipanner::getCrossfadeThreshold() {
	return crossfade_threshold;
}

void Multipanner::setCrossfadeThreshold(float t) {
	crossfade_threshold = t;
}

int Multipanner::getStrategy() {
	return strategy;
}

void Multipanner::setStrategy(int s) {
	if(strategy == s) return; //no-op.
	if(s == Lav_PANNING_STRATEGY_DELEGATE) s = Lav_PANNING_STRATEGY_STEREO;
	if(s == Lav_PANNING_STRATEGY_HRTF) hrtf_panner.reset(); //hrtf panner is stateful.
	//Note that amplitude panners are stateless, and we needn't reset them.
	//unfortunately, we do need to reconfigure other things.
	switch(s) {
		case Lav_PANNING_STRATEGY_STEREO:
		channels = 2;
		amplitude_panner.readMap(2, standard_panning_map_stereo);
		break;
		case Lav_PANNING_STRATEGY_SURROUND40:
		channels = 4;
		amplitude_panner.readMap(4, standard_panning_map_surround40);
		break;
		case Lav_PANNING_STRATEGY_SURROUND51:
		channels = 6;
		amplitude_panner.readMap(6, standard_panning_map_surround51);
		break;
		case Lav_PANNING_STRATEGY_SURROUND71:
		channels = 8;
		amplitude_panner.readMap(8, standard_panning_map_surround71);
		break;
		default: //do stereo.
		channels = 2;
		amplitude_panner.readMap(2, standard_panning_map_stereo);
		break;
	}
	strategy = s;
}


}