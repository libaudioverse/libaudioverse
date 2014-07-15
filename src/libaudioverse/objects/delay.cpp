/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <limits>
#include <memory>

class LavDelayObject: public LavObject {
	public:
	LavDelayObject(std::shared_ptr<LavDevice> device, unsigned int lines);
	~LavDelayObject();
	void process();
	protected:
	void maxDelayChanged();
	void beginInterpolation();
	void endInterpolation();
	void recomputeDelta();
	float computeSample(float pos, unsigned int line);
	//advance the line by one sample.
	void advance(unsigned int offset);
	std::vector<float*> delay_lines;
	int delay_line_length = 0;
	int write_pos = 0, line_count = 0;
	//these variables are needed for interpolation.
	float current_delay_pos = 0, target_delay_pos = 0, delta = 0, current_delay_pos_weight= 1.0f, target_delay_pos_weight = 0.0f;
	bool is_interpolating = false;
};

LavDelayObject::LavDelayObject(std::shared_ptr<LavDevice> device, unsigned int lines): LavObject(Lav_OBJTYPE_DELAY, device, lines, lines) {
	line_count = lines;
	if(lines == 0) throw LavErrorException(Lav_ERROR_RANGE);
	getProperty(Lav_DELAY_DELAY_MAX).setPostChangedCallback([this] () {maxDelayChanged();});
	getProperty(Lav_DELAY_INTERPOLATION_TIME).setPostChangedCallback([this] () {recomputeDelta();});
	getProperty(Lav_DELAY_DELAY).setPostChangedCallback([this] () {beginInterpolation();});
	maxDelayChanged(); //delegate the allocation.
	recomputeDelta(); //get delta set for the first time.
}

std::shared_ptr<LavObject> createDelayObject(std::shared_ptr<LavDevice> device, unsigned int lines) {
	auto tmp = std::make_shared<LavDelayObject>(device, lines);
	device->associateObject(tmp);
	return tmp;
}

LavDelayObject::~LavDelayObject() {
	for(auto i: delay_lines) {
		delete[] i;
	}
}

void LavDelayObject::maxDelayChanged() {
	for(auto i: delay_lines) {
		delete[] i;
	}
	delay_lines.clear();
	float delay = getProperty(Lav_DELAY_DELAY).getFloatValue();
	float maxDelay = getProperty(Lav_DELAY_DELAY_MAX).getFloatValue();
	if(maxDelay < delay) {
		delay = maxDelay;
		getProperty(Lav_DELAY_DELAY).setFloatValue(delay);
	}
	getProperty(Lav_DELAY_DELAY).setFloatRange(0.001f, maxDelay);
	if(current_delay_pos > maxDelay) {
		current_delay_pos = maxDelay;
	}
	if(target_delay_pos > maxDelay) {
		target_delay_pos = maxDelay;
	}
	unsigned int delayLineLength = (unsigned int)(maxDelay*device->getSr());
	delay_line_length = delayLineLength;
	//reallocate the lines.
	for(int i = 0; i < line_count; i++) {
		float* tmp = new float[delay_line_length]();
		delay_lines.push_back(tmp);
	}
	write_pos = 0;
}

void LavDelayObject::recomputeDelta() {
	delta = (1.0f/getProperty(Lav_DELAY_INTERPOLATION_TIME).getFloatValue())/device->getSr();
}

void LavDelayObject::beginInterpolation() {
	endInterpolation();
	is_interpolating = true;
	target_delay_pos = getProperty(Lav_DELAY_DELAY).getFloatValue();
}

void LavDelayObject::endInterpolation() {
	is_interpolating = false;
	current_delay_pos = target_delay_pos;
	current_delay_pos_weight = 1.0f;
	target_delay_pos_weight = 0.0f;
}

float LavDelayObject::computeSample(float pos, unsigned int line) {
	int position = (int)floorf(pos*device->getSr());
	float offset = pos*device->getSr()-(float)position;
	//if this is not going to leave enough room to interpolate two samples, we have to fix it.
	//this can happen sometimes when delay is close to the max and floating point inaccuracies add up.
	if(position >= delay_line_length) {
		position = delay_line_length-2;
	}
	int samp1 = ringmodi(write_pos-position, delay_line_length);
	int samp2 = ringmodi(write_pos-(position+1), delay_line_length);
	float weight1 = 1-offset, weight2 = offset;
	return weight1*delay_lines[line][samp1]+weight2*delay_lines[line][samp2];
}

void LavDelayObject::advance(unsigned int offset) {
	for(unsigned int output = 0; output < num_outputs; output++) {
		delay_lines[output][write_pos] = inputs[output][offset];
	}
	write_pos = ringmodi(write_pos+1, delay_line_length);
}

//todo:redo this whole algorithm.  We can fall to the nearest sample instead of interpolating.
void LavDelayObject::process() {
	for(unsigned int i = 0; i < block_size; i++) {
		if(is_interpolating) {
			for(unsigned int output = 0; output < num_outputs; output++) {
				float samp1 = current_delay_pos_weight*computeSample(current_delay_pos, output);
				float samp2 = target_delay_pos_weight*computeSample(target_delay_pos, output);
				float samp = samp1+samp2;
				outputs[output][i] = samp;
			}
			//Note the extra check.  For very very small delta, the condition on weight1 may trigger before the condition on weight2.
			if(current_delay_pos_weight-delta < 0.0f || target_delay_pos_weight+delta > 1.0f) {
				endInterpolation();
				advance(i);
				continue;
			}
			current_delay_pos_weight-= delta;
			target_delay_pos_weight += delta;
		}
		else {
			for(unsigned int output = 0; output < num_outputs; output++) {
				float samp = computeSample(current_delay_pos, output);
				outputs[output][i] = samp;
			}
		}
	advance(i);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createDelayObject(LavDevice* device, unsigned int lines, LavObject** destination) {
	PUB_BEGIN
	auto d = createDelayObject(incomingPointer<LavDevice>(device), lines);
	*destination = outgoingPointer(d);
	PUB_END
}
