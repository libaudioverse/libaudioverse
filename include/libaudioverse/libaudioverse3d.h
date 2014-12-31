/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#ifdef __cplusplus
extern "C" {
#endif

/**This is the interface to the 3d simulation of Libaudioverse, including its properties.*/
Lav_PUBLIC_FUNCTION LavError Lav_createSimpleEnvironmentObject(LavSimulation* sim, const char*hrtfPath, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_createSourceObject(LavSimulation* sim, LavObject* environment, LavObject** destination);

///A few properties common to most objects in the 3d simulation including all environments.
//note: reserves up to -20 before hitting Lav_ENVIRONMENT_STANDARD_PROPERTIES.
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
};

enum Lav_SOURCE_PROPERTIES {
	Lav_SOURCE_MAX_DISTANCE = -3,
	Lav_SOURCE_DISTANCE_MODEL = -4,
	Lav_SOURCE_SIZE = -5,
	Lav_SOURCE_PANNER_STRATEGY = -8,
};

enum Lav_DISTANCE_MODELS {
	Lav_DISTANCE_MODEL_LINEAR = 0, //sounds get quieter as 1-(distance/max_distance).
	Lav_DISTANCE_MODEL_EXPONENTIAL = 1, //sounds get quieter as 1/distance.
	Lav_DISTANCE_MODEL_INVERSE_SQUARE = 2, //sounds get quieter as 1/min(distance, max_distance)^2
};

#ifdef __cplusplus
}
#endif