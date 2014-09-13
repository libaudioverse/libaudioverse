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
Lav_OBJTYPE_PROPNAME
or
Lav_OBJTYPE_callbackname_CALLBACK

Furthermore, all libaudioverse properties are negative.

Values below -100 are reserved for standard callbacks and properties on all objects.
*/

enum lav_STANDARD_PROPERTIES {
	Lav_OBJECT_STATE = -100,
	Lav_OBJECT_AUTORESET = -101,
};

enum Lav_SINE_PROPERTIES {
	Lav_SINE_FREQUENCY = -1,
};

enum Lav_FILE_PROPERTIES {
	Lav_FILE_POSITION = -1,
	Lav_FILE_PITCH_BEND = -2,
	Lav_FILE_LOOPING = -3,
	//callbacks
	Lav_FILE_END_CALLBACK = -1,
};

enum Lav_PANNER_PROPERTIES {
	Lav_PANNER_AZIMUTH = -1,
	Lav_PANNER_ELEVATION = -2,
	Lav_PANNER_CHANNEL_MAP = -3,
	Lav_PANNER_SHOULD_CROSSFADE = -4,
};

enum Lav_ATTENUATOR_PROPERTIES {
	Lav_ATTENUATOR_MULTIPLIER = -1,
};

enum Lav_MIXER_PROPERTIES {
	Lav_MIXER_MAX_PARENTS = -1,
	Lav_MIXER_INPUTS_PER_PARENT = -2,
};

enum Lav_DELAY_PROPERTIES {
	Lav_DELAY_DELAY = -1,
	Lav_DELAY_DELAY_MAX = -2,
	Lav_DELAY_FEEDBACK = -3,
	Lav_DELAY_INTERPOLATION_TIME = -4,
};

enum Lav_PUSH_OBJECT_PROPERTIES {
	Lav_PUSH_THRESHOLD = -1,
	Lav_PUSH_AUDIO_CALLBACK = -2,
	Lav_PUSH_OUT_CALLBACK = -3,
};

//biquad objects.

enum Lav_BIQUAD_TYPES {
	Lav_BIQUAD_TYPE_LOWPASS = 0,
	Lav_BIQUAD_TYPE_HIGHPASS = 1,
	Lav_BIQUAD_TYPE_BANDPASS = 2,
	Lav_BIQUAD_TYPE_NOTCH = 3,
	Lav_BIQUAD_TYPE_ALLPASS = 4,
	Lav_BIQUAD_TYPE_PEAKING = 5,
	Lav_BIQUAD_TYPE_LOWSHELF = 6,
	Lav_BIQUAD_TYPE_HIGHSHELF = 7,
	Lav_BIQUAD_TYPE_IDENTITY = 8,
};

#ifdef __cplusplus
}
#endif