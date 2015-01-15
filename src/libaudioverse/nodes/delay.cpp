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
#include <libaudioverse/implementations/delayline.hpp>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
#include <math.h>

class LavDelayObject: public LavObject {
	public:
	LavDelayObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, unsigned int lineCount);
	void process();
	protected:
	void delayChanged();
	void recomputeDelta();
	unsigned int delay_line_length = 0;
	std::vector<LavCrossfadingDelayLine> lines;
};

LavDelayObject::LavDelayObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, unsigned int lineCount): LavObject(Lav_OBJTYPE_DELAY, simulation, lineCount, lineCount) {
	if(lineCount == 0) throw LavErrorException(Lav_ERROR_RANGE);
	for(unsigned int i = 0; i < lineCount; i++) lines.emplace_back(maxDelay, simulation->getSr());
	getProperty(Lav_DELAY_DELAY).setFloatRange(0.0f, maxDelay);
	getProperty(Lav_DELAY_INTERPOLATION_TIME).setPostChangedCallback([this] () {recomputeDelta();});
	getProperty(Lav_DELAY_DELAY).setPostChangedCallback([this] () {delayChanged();});
	recomputeDelta();
	delayChanged();
	//finally, set the read-only max delay.
	getProperty(Lav_DELAY_DELAY_MAX).setFloatValue(maxDelay);
}

std::shared_ptr<LavObject> createDelayObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, unsigned int lineCount) {
	auto tmp = std::shared_ptr<LavDelayObject>(new LavDelayObject(simulation, maxDelay, lineCount), LavObjectDeleter);
	simulation->associateObject(tmp);
	return tmp;
}
void LavDelayObject::recomputeDelta() {
	float time = getProperty(Lav_DELAY_INTERPOLATION_TIME).getFloatValue();
	for(auto &line: lines) line.setInterpolationTime(time);
}

void LavDelayObject::delayChanged() {
	float newDelay = getProperty(Lav_DELAY_DELAY).getFloatValue();
	for(auto &line: lines) line.setDelay(newDelay);
}

void LavDelayObject::process() {
	float feedback = getProperty(Lav_DELAY_FEEDBACK).getFloatValue();
	//optimize the common case of not having feedback.
	//the only difference between these blocks is in the advance line.
	if(feedback == 0.0f) {
		for(unsigned int output = 0; output < getOutputCount(); output++) {
			auto &line = lines[output];
			for(unsigned int i = 0; i < block_size; i++) {
				outputs[output][i] = line.computeSample();
				line.advance(inputs[output][i]);
			}
		}
	}
	else {
		for(unsigned int output = 0; output < getOutputCount(); output++) {
			auto &line = lines[output];
			for(unsigned int i = 0; i < block_size; i++) {
				outputs[output][i] = line.computeSample();
				line.advance(inputs[output][i]+outputs[output][i]*feedback);
			}
		}
	}
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createDelayObject(LavSimulation* simulation, float maxDelay, unsigned int lineCount, LavObject** destination) {
	PUB_BEGIN
	auto d = createDelayObject(incomingPointer<LavSimulation>(simulation), maxDelay, lineCount);
	*destination = outgoingPointer(d);
	PUB_END
}
