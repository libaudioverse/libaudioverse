/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#ifdef __cplusplus
extern "C" {
#endif

/**This is the interface to the 3d simulation of Libaudioverse, including its properties.*/
Lav_PUBLIC_FUNCTION LavError Lav_createWorld(LavDevice* device, LavHrtfData *hrtf, LavObject** destination);
Lav_PUBLIC_FUNCTION LavError Lav_createMonoSource(LavObject* node, LavObject* world, LavObject** destination);

enum Lav_WORLD_PROPERTIES {
	Lav_WORLD_LISTENER_ORIENTATION = 0,
	Lav_WORLD_LISTENER_POSITION = 1,
};

enum Lav_SOURCE_PROPERTIES {
	Lav_SOURCE_POSITION = 0,
};

#ifdef __cplusplus
}
#endif