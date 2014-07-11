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
	void maxDelayChanged();
	protected:
	std::vector<float*> delay_lines;
	int delay_line_length = 0;
	int write_pos = 0, line_count = 0;
	float current_delay_pos = 0;
};

LavDelayObject::LavDelayObject(std::shared_ptr<LavDevice> device, unsigned int lines): LavObject(Lav_OBJTYPE_DELAY, device, lines, lines) {
	line_count = lines;
	if(lines == 0) throw LavErrorException(Lav_ERROR_RANGE);
	getProperty(Lav_DELAY_DELAY_MAX).setPostChangedCallback([this] () {maxDelayChanged();});
	maxDelayChanged(); //delegate the allocation.
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
	getProperty(Lav_DELAY_DELAY).setFloatRange(0.0f, maxDelay);
	unsigned int delayLineLength = (unsigned int)(maxDelay*device->getSr());
	delay_line_length = delayLineLength;
	//reallocate the lines.
	for(int i = 0; i < line_count; i++) {
		float* tmp = new float[delay_line_length]();
		delay_lines.push_back(tmp);
	}
}

void LavDelayObject::process() {
	float targetDelay = getProperty(Lav_DELAY_DELAY).getFloatValue();
	float interpolationTime = getProperty(Lav_DELAY_INTERPOLATION_TIME).getFloatValue();
	float delta = interpolationTime == 0.0f ? 0.0f : 1/interpolationTime/device->getSr(); //each tick is 1/sr of a second, so we have to modify delta appropriately
	if(delta == 0.0f) current_delay_pos = targetDelay; //impossible to move, so we just jump.
	for(unsigned int i = 0; i < block_size; i++) {
		while(abs(current_delay_pos-targetDelay) <= delta && delta > (1/device->getSr())) {
			delta = delta/1.01f; //we can't just cut it off.
		}	
		current_delay_pos += copysignf(delta, targetDelay-current_delay_pos); //this makes sure we always move "toward" the target and does nothing if delta is 0.
		int pos = (int)floorf(current_delay_pos*device->getSr());
		float offset = current_delay_pos*device->getSr()-(float)pos;
		float weight1 = 1-offset;
		float weight2 = offset;
		if(pos == delay_line_length) pos--;
		int samp1 = ringmodi(write_pos-pos, delay_line_length);
		int samp2 = ringmodi(write_pos-(pos+1), delay_line_length);
		for(unsigned int output = 0; output < num_outputs; output++) {
			delay_lines[output][write_pos] = inputs[output][i];
			outputs[output][i] = weight1*delay_lines[output][samp1]+weight2*delay_lines[output][samp2];
		}
		write_pos = ringmodi(write_pos+1, delay_line_length);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createDelayObject(LavDevice* device, unsigned int lines, LavObject** destination) {
	PUB_BEGIN
	auto d = createDelayObject(incomingPointer<LavDevice>(device), lines);
	*destination = outgoingPointer(d);
	PUB_END
}
