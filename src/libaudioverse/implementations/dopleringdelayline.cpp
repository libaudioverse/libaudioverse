/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

DopleringDelayLine::DopleringDelayLine(float maxDelay, float sr): line((int)(maxDelay/sr)+1) {
	max_delay = (int)(maxDelay*sr)+1; //same as the ringbuffer above.
}

void DopleringDelayLine::setDelta(float d) {
	interpolation_delta = d;
}

void DopleringDelayLine::setDelay(float d) {
	//end any interpolating.
	delay = new_delay;
	delay_offset = 0.0f;
	new_delay = std::min<unsigned int>(d*sr, max_delay);
	//set up the new interpolation.
	interpolating = true;
	interpolating_direction = new_delay-delay; //what to add/subtract to move in the right direction.
}

float DopleringDelayLine::tick(float sample) {
	float retval = computeSample();
	advance(sample);
	return retval;
}

float DopleringDelayLine::computeSample() {
	if(interpolating == false) return line.read(delay);
	//otherwise, we are interpolating, so.
	//First, compute the samples on either side of us.
	int i1, i2;
	i1 = delay;
	i2 = delay+delay_offset; //if we are moving, we cannot move past the end.
	float w1 = 1-delay_offset, w2 = delay_offset;
	return line.read(i1)*w1+line.read(i2)*w2;
}

void DopleringDelayLine::advance(float sample) {
	//advance the line.
	line.advance(sample);
	if(interpolating == false) return;
	delay_offset += interpolation_delta;
	delay += interpolating_direction*(int)floorf(delay_offset);
	delay_offset=delay_offset-floorf(delay_offset);
	if(delay == new_delay) {
		interpolating = false;
	}
}

}