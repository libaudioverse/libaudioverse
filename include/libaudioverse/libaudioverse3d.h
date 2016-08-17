/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "libaudioverse.h"
#ifdef __cplusplus
extern "C" {
#endif

Lav_PUBLIC_FUNCTION LavError Lav_createEnvironmentNode(LavHandle serverHandle, const char*hrtfPath, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_environmentNodePlayAsync(LavHandle nodeHandle, LavHandle bufferHandle, float x, float y, float z, int isDry);
Lav_PUBLIC_FUNCTION LavError Lav_environmentNodeAddEffectSend(LavHandle nodeHandle, int channels, int isReverb, int connectByDefault, int* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createSourceNode(LavHandle serverHandle, LavHandle environmentHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_sourceNodeFeedEffect(LavHandle nodeHandle, int effect);
Lav_PUBLIC_FUNCTION LavError Lav_sourceNodeStopFeedingEffect(LavHandle nodeHandle, int effect);
Lav_PUBLIC_FUNCTION LavError Lav_sourceNodeSetPropertiesFromEnvironment(LavHandle nodeHandle);


//all environments have these properties.
enum lav_STANDARD_ENVIRONMENT_PROPERTIES {
	Lav_ENVIRONMENT_PANNING_STRATEGY,
	Lav_ENVIRONMENT_DISTANCE_MODEL,
	Lav_ENVIRONMENT_MAX_DISTANCE = -12,
	Lav_ENVIRONMENT_DEFAULT_SIZE,
	Lav_ENVIRONMENT_OUTPUT_CHANNELS,
	Lav_ENVIRONMENT_REVERB_DISTANCE,
	Lav_ENVIRONMENT_MIN_REVERB_LEVEL,
	Lav_ENVIRONMENT_MAX_REVERB_LEVEL,
	Lav_ENVIRONMENT_POSITION ,
	Lav_ENVIRONMENT_ORIENTATION,
};

enum Lav_SOURCE_PROPERTIES {
	Lav_SOURCE_MAX_DISTANCE,
	Lav_SOURCE_DISTANCE_MODEL,
	Lav_SOURCE_SIZE,
	Lav_SOURCE_REVERB_DISTANCE,
	Lav_SOURCE_PANNING_STRATEGY,
	Lav_SOURCE_HEAD_RELATIVE,
	Lav_SOURCE_MIN_REVERB_LEVEL,
	Lav_SOURCE_MAX_REVERB_LEVEL,
	Lav_SOURCE_OCCLUSION,
	Lav_SOURCE_CONTROL_PANNING,
	Lav_SOURCE_CONTROL_DISTANCE_MODEL,
	Lav_SOURCE_CONTROL_REVERB,
	Lav_SOURCE_POSITION,
	Lav_SOURCE_ORIENTATION,
};

enum Lav_DISTANCE_MODELS {
	Lav_DISTANCE_MODEL_LINEAR,
	Lav_DISTANCE_MODEL_INVERSE,
	Lav_DISTANCE_MODEL_INVERSE_SQUARE,
};

#ifdef __cplusplus
}
#endif