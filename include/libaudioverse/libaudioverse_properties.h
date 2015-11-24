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

Furthermore, all libaudioverse properties are negative.

Values below -100 are reserved for standard events and properties on all objects.
*/

enum lav_STANDARD_PROPERTIES {
	Lav_NODE_STATE = -100,
	Lav_NODE_MUL = -101,
	Lav_NODE_ADD = -102,
	Lav_NODE_CHANNEL_INTERPRETATION = -104,
};

enum Lav_CHANNEL_INTERPRETATIONS {
	Lav_CHANNEL_INTERPRETATION_DISCRETE = 0,
	Lav_CHANNEL_INTERPRETATION_SPEAKERS = 1,
};

enum Lav_OSCILLATOR_PROPERTIES {
	Lav_OSCILLATOR_FREQUENCY = -200,
	Lav_OSCILLATOR_PHASE = -201,
	Lav_OSCILLATOR_FREQUENCY_MULTIPLIER = -202,
};

enum lav_SQUARE_PROPERTIES {
	Lav_SQUARE_HARMONICS = -1,
	Lav_SQUARE_DUTY_CYCLE = -2,
};

enum lav_TRIANGLE_PROPERTIES {
	Lav_TRIANGLE_HARMONICS = -3,
};

enum lav_SAW_PROPERTIES {
	Lav_SAW_HARMONICS = -3,
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
	Lav_PANNER_HAS_LFE = -6,
	Lav_PANNER_SKIP_CENTER = -7,
	Lav_PANNER_HAS_CENTER = -8,
	Lav_PANNER_STRATEGY = -9,
	Lav_PANNER_PASSTHROUGH = -10,
};

enum Lav_PANNER_BANK_PROPERTIES {
	Lav_PANNER_BANK_SPREAD = -20,
	Lav_PANNER_BANK_COUNT = -21,
	Lav_PANNER_BANK_IS_CENTERED =-22,
};

enum Lav_PANNING_STRATEGIES {
	Lav_PANNING_STRATEGY_HRTF = 0,
	Lav_PANNING_STRATEGY_STEREO = 1,
	Lav_PANNING_STRATEGY_SURROUND40 = 2,
	Lav_PANNING_STRATEGY_SURROUND51 = 3,
	Lav_PANNING_STRATEGY_SURROUND71 = 4,
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
	Lav_FDN_OUTPUT_GAINS = -2,
	Lav_FDN_DELAYS = -3,
	Lav_FDN_MATRIX = -4,
	Lav_FDN_FILTER_TYPES = -5,
	Lav_FDN_FILTER_FREQUENCIES = -6,
};

enum Lav_FDN_FILTER_TYPES {
	Lav_FDN_FILTER_TYPE_DISABLED = 0,
	Lav_FDN_FILTER_TYPE_LOWPASS = 1,
	Lav_FDN_FILTER_TYPE_HIGHPASS = 2,
};

enum Lav_BUFFER_PROPERTIES {
	Lav_BUFFER_BUFFER = -1,
	Lav_BUFFER_POSITION = -2,
	Lav_BUFFER_RATE = -3,
	Lav_BUFFER_LOOPING = -4,
	Lav_BUFFER_ENDED_COUNT = -5,
};

enum Lav_CONVOLVER_PROPERTIES {
	Lav_CONVOLVER_IMPULSE_RESPONSE = -1,
};

enum Lav_THREE_BAND_EQ_PROPERTIES {
	Lav_THREE_BAND_EQ_HIGHBAND_DBGAIN = -1,
	Lav_THREE_BAND_EQ_HIGHBAND_FREQUENCY = -2,
	Lav_THREE_BAND_EQ_MIDBAND_DBGAIN = -3,
	Lav_THREE_BAND_EQ_LOWBAND_DBGAIN = -4,
	Lav_THREE_BAND_EQ_LOWBAND_FREQUENCY= -5,
};

enum Lav_FILTERED_DELAY_PROPERTIES {
	Lav_FILTERED_DELAY_DELAY = -1,
	Lav_FILTERED_DELAY_FEEDBACK = -2,
	Lav_FILTERED_DELAY_INTERPOLATION_TIME = -3,
	Lav_FILTERED_DELAY_DELAY_MAX = -4,
	Lav_FILTERED_DELAY_FILTER_TYPE = -5,
	Lav_FILTERED_DELAY_Q = -6,
	Lav_FILTERED_DELAY_FREQUENCY = -7,
	Lav_FILTERED_DELAY_DBGAIN = -8,
};

enum Lav_CROSSFADER_PROPERTIES {
	Lav_CROSSFADER_CURRENT_INPUT = -1,
	Lav_CROSSFADER_TARGET_INPUT = -2,
	Lav_CROSSFADER_IS_CROSSFADING = -3,
};

enum Lav_ONE_POLE_FILTER_PROPERTIES {
	Lav_ONE_POLE_FILTER_FREQUENCY = -1,
	Lav_ONE_POLE_FILTER_IS_HIGHPASS = -2,
};

enum Lav_FIRST_ORDER_FILTER_PROPERTIES {
	Lav_FIRST_ORDER_FILTER_POLE = -1,
	Lav_FIRST_ORDER_FILTER_ZERO = -2,
};

enum Lav_ALLPASS_OPROPERTIES {
	Lav_ALLPASS_DELAY_SAMPLES = -1,
	Lav_ALLPASS_DELAY_SAMPLES_MAX = -2,
	Lav_ALLPASS_INTERPOLATION_TIME = -3,
	Lav_ALLPASS_COEFFICIENT = -4,
};

enum Lav_FDN_REVERB_PROPERTIES {
	Lav_FDN_REVERB_T60 = -1,
	Lav_FDN_REVERB_CUTOFF_FREQUENCY = -2,
	Lav_FDN_REVERB_DENSITY = -3,
	Lav_FDN_REVERB_DELAY_MODULATION_DEPTH = -4,
	Lav_FDN_REVERB_DELAY_MODULATION_FREQUENCY = -5,
};

enum Lav_BLIT_PROPERTIES {
	Lav_BLIT_HARMONICS = -1,
	Lav_BLIT_SHOULD_NORMALIZE = -4,
};

enum Lav_LEAKY_INTEGRATOR_PROPERTIES {
	Lav_LEAKY_INTEGRATOR_LEAKYNESS = -1,
};

#ifdef __cplusplus
}
#endif