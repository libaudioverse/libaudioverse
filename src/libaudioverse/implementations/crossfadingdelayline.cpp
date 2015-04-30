/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

CrossfadingDelayLine::CrossfadingDelayLine(float maxDelay, float sr): line((int)(sr*maxDelay)+1) {
	this->sr = sr;
}

void CrossfadingDelayLine::setDelay(float delay) {
	int newDelay = (unsigned int)(delay*sr);
	if(newDelay >= line.getLength()) newDelay = line.getLength()-1;
	new_delay = newDelay;
	is_interpolating = true;
	//we do not screw with the weights.
	//if we are already interpolating, there is no good option, but suddenly moving back is worse.
}

void CrossfadingDelayLine::setInterpolationTime(float t) {
	interpolation_delta = 1.0/(t*sr);
}

float CrossfadingDelayLine::computeSample() {
	if(is_interpolating) return weight1*line.read(delay)+weight2*line.read(new_delay);
	return line.read(delay);
}

void CrossfadingDelayLine::advance(float sample) {
	line.advance(sample);
	if(is_interpolating) {
		weight1 -= interpolation_delta;
		if(weight1 < 0.0f) weight1 = 0.0f;
		weight2 += interpolation_delta;
		if(weight2 >= 1.0f) {
			weight1 = 1.0f;
			weight2 = 0.0f;
			delay = new_delay;
			is_interpolating = false;
		}
	}
}
void CrossfadingDelayLine::write(float delay, float value) {
	int index = (int)(delay*sr);
	line.write(index, value);
}

void CrossfadingDelayLine::add(float delay, float value) {
	int index = (int)(delay*sr);
	line.write(index, value);
}

void CrossfadingDelayLine::reset() {
	weight1 = 1.0f;
	weight2 = 0.0f;
	is_interpolating = false;
	line.reset();
}

}