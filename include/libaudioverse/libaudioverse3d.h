/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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