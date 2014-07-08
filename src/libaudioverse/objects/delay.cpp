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

class LavDelay: public LavObject {
	public:
	LavDelay(std::shared_ptr<LavDevice> device, unsigned int lines);
	~LavDelay();
	void process();
	void maxDelayChanged();
	protected:
	std::vector<float*> delay_lines;
	int delay_line_length = 0;
	int write_pos = 0, line_count = 0;
};

LavDelay::LavDelay(std::shared_ptr<LavDevice> device, unsigned int lines): LavObject(Lav_OBJTYPE_DELAY, device, lines, lines) {
	line_count = lines;
	if(lines == 0) throw LavErrorException(Lav_ERROR_RANGE);
	getProperty(Lav_DELAY_DELAY_MAX).setPostChangedCallback([this] () {maxDelayChanged();});
	maxDelayChanged(); //delegate the allocation.
}

std::shared_ptr<LavObject> createDelayObject(std::shared_ptr<LavDevice> device, unsigned int lines) {
	auto tmp = std::make_shared<LavDelay>(device, lines);
	device->associateObject(tmp);
	return tmp;
}

LavDelay::~LavDelay() {
	for(auto i: delay_lines) {
		delete[] i;
	}
}

void LavDelay::maxDelayChanged() {
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

void LavDelay::process() {
	float delay = getProperty(Lav_DELAY_DELAY).getFloatValue()*device->getSr();
	int readOffset = (unsigned int)floorf(delay);
	float offset = delay-floorf(delay);
	//this prevents us from actually trying to read the future.
	if(readOffset >= delay_line_length) {
		readOffset = delay_line_length-1;
	}
	for(unsigned int i = 0; i < block_size; i++) {
		int samp1 = -readOffset, samp2 = -(readOffset+1);
		samp1 = ringmodi(samp1+write_pos, delay_line_length);
		samp2 = ringmodi(samp2+write_pos, delay_line_length);
		float weight1 = 1.0f-offset;
		float weight2 = offset;
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
