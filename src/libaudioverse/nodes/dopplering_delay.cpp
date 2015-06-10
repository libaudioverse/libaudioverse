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

namespace libaudioverse_implementation {

class DoppleringDelayNode: public Node {
	public:
	DoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int lineCount);
	~DoppleringDelayNode();
	void process();
	protected:
	void delayChanged();
	void recomputeDelta();
	unsigned int delay_line_length = 0;
	unsigned int line_count;
	DoppleringDelayLine **lines;
};

DoppleringDelayNode::DoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int lineCount): Node(Lav_OBJTYPE_DOPPLERING_DELAY_NODE, simulation, lineCount, lineCount), line_count(lineCount) {
	if(lineCount == 0) throw LavErrorException(Lav_ERROR_RANGE);
	lines = new DoppleringDelayLine*[lineCount];
	for(unsigned int i = 0; i < lineCount; i++) lines[i] = new DoppleringDelayLine(maxDelay, simulation->getSr());
	getProperty(Lav_DELAY_DELAY).setFloatRange(0.0f, maxDelay);
	getProperty(Lav_DELAY_DELTA).setPostChangedCallback([this] () {recomputeDelta();});
	getProperty(Lav_DELAY_DELAY).setPostChangedCallback([this] () {delayChanged();});
	recomputeDelta();
	delayChanged();
	//finally, set the read-only max delay.
	getProperty(Lav_DELAY_DELAY_MAX).setFloatValue(maxDelay);
	appendInputConnection(0, lineCount);
	appendOutputConnection(0, lineCount);
}

std::shared_ptr<Node> createDoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int lineCount) {
	auto tmp = std::shared_ptr<DoppleringDelayNode>(new DoppleringDelayNode(simulation, maxDelay, lineCount), ObjectDeleter(simulation));
	simulation->associateNode(tmp);
	return tmp;
}

DoppleringDelayNode::~DoppleringDelayNode() {
	for(unsigned int i = 0; i < line_count; i++) delete lines[i];
	delete[] lines;
}

void DoppleringDelayNode::recomputeDelta() {
	float delta = getProperty(Lav_DELAY_DELTA).getFloatValue();
	for(unsigned int i = 0; i < line_count; i++) lines[i]->setDelta(delta);
}

void DoppleringDelayNode::delayChanged() {
	float newDelay = getProperty(Lav_DELAY_DELAY).getFloatValue();
	for(unsigned int i = 0; i < line_count; i++) lines[i]->setDelay(newDelay);
}

void DoppleringDelayNode::process() {
	//No feedback is in this variant.
	for(int output = 0; output < num_output_buffers; output++) {
		auto &d = *lines[output];
		for(int i = 0; i < block_size; i++) {
			output_buffers[output][i] = d.tick(input_buffers[output][i]);
		}
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createDoppleringDelayNode(LavHandle simulationHandle, float maxDelay, unsigned int lineCount, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto d = createDoppleringDelayNode(simulation, maxDelay, lineCount);
	*destination = outgoingObject(d);
	PUB_END
}

}