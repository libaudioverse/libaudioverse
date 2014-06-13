/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#ifdef __cplusplus
extern "C" {
#endif

/**This file contains the property enums for Libaudioverse.

It is worth keeping separate because it will grow rapidly and contain documentation comments and etc.

The standard for naming is:
Lav_NODETYPE_PROPNAME
*/

enum Lav_SINE_PROPERTIES {
	Lav_SINE_FREQUENCY = 0,
};

enum Lav_FILE_PROPERTIES {
	Lav_FILE_POSITION = 0,
	Lav_FILE_PITCH_BEND = 1,
};

enum Lav_HRTF_PROPERTIES {
	Lav_HRTF_AZIMUTH = 0,
	Lav_HRTF_ELEVATION = 1,
};

enum Lav_ATTENUATOR_PROPERTIES {
	Lav_ATTENUATOR_MULTIPLIER = 0,
};

#ifdef __cplusplus
}
#endif