/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/dopplering_delay.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <memory>

namespace libaudioverse_implementation {

DoppleringDelayNode::DoppleringDelayNode(std::shared_ptr<Server> server, float maxDelay, int channels): Node(Lav_OBJTYPE_DOPPLERING_DELAY_NODE, server, channels, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "lineCount must be greater than 0.");
	this->channels = channels;
	lines = new DoppleringDelayLine*[channels]();
	for(unsigned int i = 0; i < channels; i++) lines[i] = new DoppleringDelayLine(maxDelay, server->getSr());
	getProperty(Lav_DELAY_DELAY).setFloatRange(0.0f, maxDelay);
	getProperty(Lav_DELAY_DELAY_MAX).setFloatValue(maxDelay);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createDoppleringDelayNode(std::shared_ptr<Server> server, float maxDelay, int channels) {
	return standardNodeCreation<DoppleringDelayNode>(server, maxDelay, channels);
}

DoppleringDelayNode::~DoppleringDelayNode() {
	for(int i = 0; i < channels; i++) delete lines[i];
	delete[] lines;
}

void DoppleringDelayNode::recomputeDelta() {
	float time = getProperty(Lav_DELAY_INTERPOLATION_TIME).getFloatValue();
	for(int i = 0; i < channels; i++) lines[i]->setInterpolationTime(time);
}

void DoppleringDelayNode::delayChanged() {
	float newDelay = getProperty(Lav_DELAY_DELAY).getFloatValue();
	for(int i = 0; i < channels; i++) lines[i]->setDelay(newDelay);
}

void DoppleringDelayNode::process() {
	if(werePropertiesModified(this, Lav_DELAY_DELAY)) delayChanged();
	if(werePropertiesModified(this, Lav_DELAY_INTERPOLATION_TIME)) recomputeDelta();
	for(int output = 0; output < num_output_buffers; output++) {
		auto &line = *lines[output];
		for(int i = 0; i < block_size; i++) output_buffers[output][i] = line.tick(input_buffers[output][i]);
	}
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createDoppleringDelayNode(LavHandle serverHandle, float maxDelay, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server =incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto d = createDoppleringDelayNode(server, maxDelay, channels);
	*destination = outgoingObject(d);
	PUB_END
}

}