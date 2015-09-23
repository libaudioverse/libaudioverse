/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#ifdef __cplusplus
extern "C" {
#endif

/**This is the interface to the 3d simulation of Libaudioverse, including its properties.*/

Lav_PUBLIC_FUNCTION LavError Lav_createEnvironmentNode(LavHandle simulationHandle, const char*hrtfPath, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_environmentNodePlayAsync(LavHandle nodeHandle, LavHandle bufferHandle, float x, float y, float z, int isDry);
Lav_PUBLIC_FUNCTION LavError Lav_environmentNodeAddEffectSend(LavHandle nodeHandle, int channels, int isReverb, int connectByDefault, int* destination);

Lav_PUBLIC_FUNCTION LavError Lav_createSourceNode(LavHandle simulationHandle, LavHandle environmentHandle, LavHandle* destination);
Lav_PUBLIC_FUNCTION LavError Lav_sourceNodeFeedEffect(LavHandle nodeHandle, int effect);
Lav_PUBLIC_FUNCTION LavError Lav_sourceNodeStopFeedingEffect(LavHandle nodeHandle, int effect);

//A few properties common to most objects in the 3d simulation including all environments.
enum Lav_3D_PROPERTIES {
	Lav_3D_ORIENTATION = -1, //float6 consisting of an at followed by an up vector.
	Lav_3D_POSITION = -2, //float3, consisting of the position of the object.
};

//all environments have these properties.
enum lav_STANDARD_ENVIRONMENT_PROPERTIES {
	Lav_ENVIRONMENT_DEFAULT_PANNER_STRATEGY = -8,
	Lav_ENVIRONMENT_DEFAULT_DISTANCE_MODEL = -11,
	Lav_ENVIRONMENT_DEFAULT_MAX_DISTANCE = -12,
	Lav_ENVIRONMENT_DEFAULT_SIZE = -13,
	Lav_ENVIRONMENT_OUTPUT_CHANNELS = -14,
	Lav_ENVIRONMENT_DEFAULT_REVERB_DISTANCE = -15,
};

enum Lav_SOURCE_PROPERTIES {
	Lav_SOURCE_MAX_DISTANCE = -3,
	Lav_SOURCE_DISTANCE_MODEL = -4,
	Lav_SOURCE_SIZE = -5,
	Lav_SOURCE_REVERB_DISTANCE = -6,
	Lav_SOURCE_PANNER_STRATEGY = -8,
	Lav_SOURCE_HEAD_RELATIVE = -9,
	Lav_SOURCE_MIN_REVERB_LEVEL = -10,
	Lav_SOURCE_MAX_REVERB_LEVEL = -11,
	Lav_SOURCE_OCCLUSION = -12,
};

enum Lav_DISTANCE_MODELS {
	Lav_DISTANCE_MODEL_LINEAR = 0, //sounds get quieter as 1-(distance/max_distance).
	Lav_DISTANCE_MODEL_EXPONENTIAL = 1, //sounds get quieter as 1/distance.
	Lav_DISTANCE_MODEL_INVERSE_SQUARE = 2, //sounds get quieter as 1/min(distance, max_distance)^2
};

#ifdef __cplusplus
}
#endif