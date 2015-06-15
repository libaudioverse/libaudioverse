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
or
Lav_NODETYPE_callbackname_EVENT

Furthermore, all libaudioverse properties are negative.

Values below -100 are reserved for standard events and properties on all objects.
*/

enum lav_STANDARD_PROPERTIES {
	Lav_NODE_STATE = -100,
	Lav_NODE_AUTORESET = -101,
	Lav_NODE_MUL = -102,
	Lav_NODE_ADD = -103,
	Lav_NODE_CHANNEL_INTERPRETATION = -104,
};

enum Lav_CHANNEL_INTERPRETATIONS {
	Lav_CHANNEL_INTERPRETATION_DISCRETE = 0,
	Lav_CHANNEL_INTERPRETATION_SPEAKERS = 1,
};

enum Lav_SINE_PROPERTIES {
	Lav_SINE_FREQUENCY = -1,
	Lav_SINE_PHASE =-2,
};

enum lav_SQUARE_PROPERTIES {
	Lav_SQUARE_FREQUENCY = -1,
	Lav_SQUARE_DUTY_CYCLE = -2,
	Lav_SQUARE_PHASE = -3,
};

enum Lav_NOISE_PROPERTIES {
	Lav_NOISE_NOISE_TYPE = -1,
	Lav_NOISE_SHOULD_NORMALIZE = -2,
};

enum Lav_NOISE_TYPES {
	Lav_NOISE_TYPE_WHITE = 0,
	Lav_NOISE_TYPE_PINK = 1,
	Lav_NOISE_TYPE_BROWN = 2,
};

enum Lav_PANNER_PROPERTIES {
	Lav_PANNER_AZIMUTH = -1,
	Lav_PANNER_ELEVATION = -2,
	Lav_PANNER_CHANNEL_MAP = -3,
	Lav_PANNER_SHOULD_CROSSFADE = -4,
	Lav_PANNER_SKIP_LFE = -5,
	Lav_PANNER_SKIP_CENTER = -6,
	Lav_PANNER_STRATEGY = -7,
	Lav_PANNER_SPEED_OF_SOUND = -8,
	Lav_PANNER_DISTANCE = -9,
	Lav_PANNER_HEAD_WIDTH = -10,
	Lav_PANNER_EAR_POSITION = -11,
};

enum Lav_PANNING_STRATEGIES {
	Lav_PANNING_STRATEGY_HRTF = 0,
	Lav_PANNING_STRATEGY_STEREO = 1,
	Lav_PANNING_STRATEGY_SURROUND51 = 2,
	Lav_PANNING_STRATEGY_SURROUND71 = 3,
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

enum Lav_PUSH_NODE_PROPERTIES {
	Lav_PUSH_THRESHOLD = -1,
	Lav_PUSH_AUDIO_EVENT = -2,
	Lav_PUSH_OUT_EVENT = -3,
};

//biquad objects.

enum Lav_BIQUAD_PROPERTIES {
	Lav_BIQUAD_FILTER_TYPE = -1,
	Lav_BIQUAD_Q = -2,
	Lav_BIQUAD_FREQUENCY = -3,
	Lav_BIQUAD_DBGAIN = -4,
};

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

//this is for feedback delay networks. We shorten because otherwise it would be insane to actually use these.
enum Lav_FEEDBACK_DELAY_NETWORK_PROPERTIES {
	Lav_FDN_MAX_DELAY = -1,
	Lav_FDN_INTERPOLATION_TIME = -2,
};

enum Lav_BUFFER_PROPERTIES {
	Lav_BUFFER_BUFFER = -1,
	Lav_BUFFER_POSITION = -2,
	Lav_BUFFER_PITCH_BEND = -3,
	Lav_BUFFER_LOOPING = -4,
	Lav_BUFFER_END_EVENT = -1,
};

enum Lav_CONVOLVER_PROPERTIES {
	Lav_CONVOLVER_IMPULSE_RESPONSE = -1,
};

#ifdef __cplusplus
}
#endif