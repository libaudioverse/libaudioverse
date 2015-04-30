/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
#include <math.h>

class DelayNode: public Node {
	public:
	DelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int lineCount);
	void process();
	protected:
	void delayChanged();
	void recomputeDelta();
	unsigned int delay_line_length = 0;
	std::vector<CrossfadingDelayLine> lines;
};

DelayNode::DelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int lineCount): Node(Lav_OBJTYPE_DELAY_NODE, simulation, lineCount, lineCount) {
	if(lineCount == 0) throw LavErrorException(Lav_ERROR_RANGE);
	for(unsigned int i = 0; i < lineCount; i++) lines.emplace_back(maxDelay, simulation->getSr());
	getProperty(Lav_DELAY_DELAY).setFloatRange(0.0f, maxDelay);
	getProperty(Lav_DELAY_INTERPOLATION_TIME).setPostChangedCallback([this] () {recomputeDelta();});
	getProperty(Lav_DELAY_DELAY).setPostChangedCallback([this] () {delayChanged();});
	recomputeDelta();
	delayChanged();
	//finally, set the read-only max delay.
	getProperty(Lav_DELAY_DELAY_MAX).setFloatValue(maxDelay);
	appendInputConnection(0, lineCount);
	appendOutputConnection(0, lineCount);
}

std::shared_ptr<Node> createDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int lineCount) {
	auto tmp = std::shared_ptr<DelayNode>(new DelayNode(simulation, maxDelay, lineCount), ObjectDeleter(simulation));
	simulation->associateNode(tmp);
	return tmp;
}

void DelayNode::recomputeDelta() {
	float time = getProperty(Lav_DELAY_INTERPOLATION_TIME).getFloatValue();
	for(auto &line: lines) line.setInterpolationTime(time);
}

void DelayNode::delayChanged() {
	float newDelay = getProperty(Lav_DELAY_DELAY).getFloatValue();
	for(auto &line: lines) line.setDelay(newDelay);
}

void DelayNode::process() {
	float feedback = getProperty(Lav_DELAY_FEEDBACK).getFloatValue();
	//optimize the common case of not having feedback.
	//the only difference between these blocks is in the advance line.
	if(feedback == 0.0f) {
		for(unsigned int output = 0; output < num_output_buffers; output++) {
			auto &line = lines[output];
			for(unsigned int i = 0; i < block_size; i++) {
				output_buffers[output][i] = line.computeSample();
				line.advance(input_buffers[output][i]);
			}
		}
	}
	else {
		for(unsigned int output = 0; output < num_output_buffers; output++) {
			auto &line = lines[output];
			for(unsigned int i = 0; i < block_size; i++) {
				output_buffers[output][i] = line.computeSample();
				line.advance(input_buffers[output][i]+output_buffers[output][i]*feedback);
			}
		}
	}
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createDelayNode(LavHandle simulationHandle, float maxDelay, unsigned int lineCount, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto d = createDelayNode(simulation, maxDelay, lineCount);
	*destination = outgoingObject(d);
	PUB_END
}
