/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "private_structs3d.hpp"
struct LavSource;
struct LavHrtfData;

struct LavWorld {
	LavSource **sources;
	unsigned int num_sources, max_sources;
	TmTransform camera_transform; //the camera transform.
	LavObject* mixer, *limiter;
	LavHrtfData *hrtf;
};

LavError worldAssociateSource(LavWorld* world, LavSource* source);
