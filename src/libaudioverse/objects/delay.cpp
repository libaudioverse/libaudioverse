/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <math.h>

class LavDelayObject: public LavObject {
	public:
	LavDelayObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, unsigned int lineCount);
	~LavDelayObject();
	void process();
	protected:
	void delayChanged();
	void recomputeDelta();
	unsigned int delay_line_length = 0;
	float** lines = nullptr;
	unsigned int read_head = 0, new_read_head = 0, write_head = 1; //if write_head=0, delay wraps
	float read_head_weight = 1.0f, new_read_head_weight = 0.0f, delta = 1.0f;
	bool interpolating = false;
	unsigned int line_count;
};

LavDelayObject::LavDelayObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, unsigned int lineCount): LavObject(Lav_OBJTYPE_DELAY, simulation, lineCount, lineCount) {
	if(lineCount == 0) throw LavErrorException(Lav_ERROR_RANGE);
	line_count = lineCount;
	//we always offset all delays by exactly 1 sample.
	//this avoids an ambiguity for when read_head=write_head that causes the delay to wrap to the maximum.
	//We could fix this by reading first, but this delay line supports feedback, which then breaks.
	delay_line_length = (unsigned int)(maxDelay*simulation->getSr());
	if(delay_line_length == 0) throw LavErrorException(Lav_ERROR_RANGE);
	delay_line_length++; //the extra sample.
	lines = new float*[lineCount];
	for(unsigned int i = 0; i < lineCount; i++) {
		lines[i] = new float[delay_line_length]();
	}
	getProperty(Lav_DELAY_DELAY).setFloatRange(0.0f, maxDelay);
	getProperty(Lav_DELAY_INTERPOLATION_TIME).setPostChangedCallback([this] () {recomputeDelta();});
	getProperty(Lav_DELAY_DELAY).setPostChangedCallback([this] () {delayChanged();});
	recomputeDelta();
	//finally, set the read-only max delay.
	getProperty(Lav_DELAY_DELAY_MAX).setFloatValue(maxDelay);
}

std::shared_ptr<LavObject> createDelayObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, unsigned int lineCount) {
	auto tmp = std::make_shared<LavDelayObject>(simulation, maxDelay, lineCount);
	simulation->associateObject(tmp);
	return tmp;
}

LavDelayObject::~LavDelayObject() {
	for(unsigned int i = 0; i < line_count; i++) delete[] lines[i];
	delete[] lines;
}

void LavDelayObject::recomputeDelta() {
	delta = (1.0f/(getProperty(Lav_DELAY_INTERPOLATION_TIME).getFloatValue())/simulation->getSr());
}

void LavDelayObject::delayChanged() {
	//get the new read head position to the nearest sample.
	unsigned int newReadHeadPos = (unsigned int)(getProperty(Lav_DELAY_DELAY).getFloatValue()*simulation->getSr()) + 1; //+1 because we offset by one sample.
	if(newReadHeadPos >= delay_line_length) newReadHeadPos = delay_line_length - 1;
	newReadHeadPos = ringmodi(write_head-newReadHeadPos, delay_line_length);
	//we set this as the new delay line position.
	new_read_head = newReadHeadPos;
	//the new read head's weight gets reset to 0.
	new_read_head_weight = 0.0f;
	//we leave the old one alone, in order to protect against rapid changes.
	interpolating = true;
}

void LavDelayObject::process() {
	float feedback = getProperty(Lav_DELAY_FEEDBACK).getFloatValue();
	for(unsigned int i = 0; i < block_size; i++) {
		for(unsigned int line = 0; line < line_count; line++) {
			float outSample = lines[line][read_head]*read_head_weight+lines[line][new_read_head]*new_read_head_weight;
			lines[line][write_head] = inputs[line][i]+outSample*feedback;
			outputs[line][i] = outSample;
		}
		//advance both pointers one sample.
		read_head = ringmodi(read_head+1, delay_line_length);
		new_read_head = ringmodi(new_read_head+1, delay_line_length);
		write_head = ringmodi(write_head+1, delay_line_length);
		//update the weights.
		if(interpolating) {
			read_head_weight = fmin(0.0f, read_head_weight - delta);
			new_read_head_weight = fmax(1.0f, new_read_head_weight+delta);
			if(new_read_head_weight == 1.0f) { //finished interpolating.
				std::swap(read_head, new_read_head);
				std::swap(read_head_weight, new_read_head_weight);
				interpolating = false;	
			}
		}
	//and output the sample.
	}
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createDelayObject(LavSimulation* simulation, float maxDelay, unsigned int lineCount, LavObject** destination) {
	PUB_BEGIN
	auto d = createDelayObject(incomingPointer<LavSimulation>(simulation), maxDelay, lineCount);
	*destination = outgoingPointer(d);
	PUB_END
}
