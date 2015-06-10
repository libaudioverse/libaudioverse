/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/private/memory.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

DoppleringDelayLine::DoppleringDelayLine(float maxDelay, float sr) {
	max_delay = (int)(maxDelay*sr)+1; //same as the ringbuffer above.
	this->sr = sr;
	line = allocArray<float>(max_delay);
}

DoppleringDelayLine::~DoppleringDelayLine() {
	freeArray(line);
}

void DoppleringDelayLine::setDelta(float d) {
	delta = d;
}

void DoppleringDelayLine::setDelay(float d) {
	//end any interpolating.
	read_head  =new_read_head;
	new_read_head = ringmodi(write_head-std::min<unsigned int>(d*sr, max_delay), max_delay);
	//set up the new interpolation.
	interpolating = true;
	interpolating_direction = read_head-new_read_head < 0.0 ? -1 : 1;
	current_delta = delta;
}

float DoppleringDelayLine::tick(float sample) {
	float retval = computeSample();
	advance(sample);
	return retval;
}

float DoppleringDelayLine::computeSample() {
	float w1, w2;
	w2 = read_head-floorf(read_head);
	w1 = 1-w2;
	int i1, i2;
	i1 = (int)read_head;
	i2 = i1+1;
	//i1 is guaranteed to always be okay per the advance function, but i2 can go past the end.
	i2 = ringmodi(i2, max_delay);
	return w1*line[i1]+w2*line[i2];
}

void DoppleringDelayLine::advance(float sample) {
	//add 1 to the write head and write our sample.
	write_head=(write_head+1)%max_delay;
	line[write_head] = sample;
	if(interpolating == false) {
		read_head += 1;
		read_head=ringmod(read_head, max_delay);
		return;
	}
	//otherwise, we're interpolating, the new read head gets 1 and the read head gets delta.
	read_head+= current_delta*interpolating_direction;
	read_head=ringmod(read_head, max_delay);
	new_read_head=ringmod(new_read_head+1, max_delay);
	double dist = abs(read_head-new_read_head);
	//we might be able to stop. We define stopping as less than a hundredth of a sample.
	if(dist <=current_delta) {
		read_head =new_read_head;
		interpolating = false;
		return;
	}
}

}