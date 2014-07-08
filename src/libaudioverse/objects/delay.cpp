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
#include <limits>
#include <memory>

class LavDelay: public LavObject {
	LavDelay(std::shared_ptr<LavDevice> device, unsigned int lines);
	~LavDelay();
	void process();
	void maxDelayChanged();
	protected:
	std::vector<float*> delay_lines;
	unsigned int delay_line_length;
};

LavDelay::LavDelay(std::shared_ptr<LavDevice> device, unsigned int lines): LavObject(Lav_OBJTYPE_DELAY, device, lines, lines) {
	if(lines == 0) throw LavErrorException(Lav_ERROR_RANGE);
	getProperty(Lav_DELAY_DELAY_MAX).setPostChangedCallback([this] () {maxDelayChanged();});
	maxDelayChanged(); //delegate the allocation.
}

LavDelay::~LavDelay() {
	for(auto i: delay_lines) {
		delete[] i;
	}
}

void LavDelay::maxDelayChanged() {
	unsigned int lineCount = delay_lines.size();
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
	for(unsigned int i = 0; i < lineCount; i++) {
		float* tmp = new float[delay_line_length];
		delay_lines.push_back(tmp);
	}
}

void LavDelay::process() {
	float delay = getProperty(Lav_DELAY_DELAY).getFloatValue();
	unsigned int sampleTarget = (unsigned int)floorf(delay);
	float offset = delay-floorf(delay);
	unsigned int samp1 = sampleTarget;
	unsigned int samp2 = samp1+1;
	//we can't wrap past the end; if the delay is past the last sample, we need to just cut it off.
	//this can't happen often, but floating point is a nightmare and this is one of those cases.
		if(samp2 > delay_line_length) samp2 = samp1;
	//the weights.
	float weight1 = 1-offset, weight2 = offset;
	for(unsigned output = 0; output < num_outputs; output++) {
		for(unsigned int i = 0; i < block_size; i++) {
			outputs[output][i] = weight1*delay_lines[output][samp1]+weight2*delay_lines[output][samp2];
		}
	}
}
