/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_structs.hpp"
#include <transmat.h>

struct LavSource;
struct LavWorld;
/**Structs for 3d worlds.*/
struct LavSource {
	LavObject base;
	TmVector position;
	LavObject* data_node, *panner_node, *attenuator_node;
	struct LavWorld *world;
};
typedef struct LavSource LavSource;

struct LavWorld {
	LavObject base;
	LavSource **sources;
	unsigned int num_sources, max_sources;
	TmTransform camera_transform; //the camera transform.
	LavObject* mixer, *limiter;
	LavHrtfData *hrtf;
};

typedef struct LavWorld LavWorld;
