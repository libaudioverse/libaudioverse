/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/blit.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/implementations/blit.hpp>

namespace libaudioverse_implementation {

BlitNode::BlitNode(std::shared_ptr<Server> server): Node(Lav_OBJTYPE_BLIT_NODE, server, 0, 1), oscillator(server->getSr()) {
	appendOutputConnection(0, 1);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createBlitNode(std::shared_ptr<Server> server) {
	return standardNodeCreation<BlitNode>(server);
}

void BlitNode::process() {
	if(werePropertiesModified(this, Lav_OSCILLATOR_PHASE)) {
		oscillator.setPhase(oscillator.getPhase()+getProperty(Lav_OSCILLATOR_PHASE).getFloatValue());
	}
	if(werePropertiesModified(this, Lav_BLIT_HARMONICS)) oscillator.setHarmonics(getProperty(Lav_BLIT_HARMONICS).getIntValue());
	if(werePropertiesModified(this, Lav_OSCILLATOR_FREQUENCY)) oscillator.setFrequency(getProperty(Lav_OSCILLATOR_FREQUENCY).getFloatValue());
	if(werePropertiesModified(this, Lav_BLIT_SHOULD_NORMALIZE)) oscillator.setShouldNormalize(getProperty(Lav_BLIT_SHOULD_NORMALIZE).getIntValue() == 1);
	auto &freq = getProperty(Lav_OSCILLATOR_FREQUENCY);
	auto &freqMul = getProperty(Lav_OSCILLATOR_FREQUENCY_MULTIPLIER);
	if(freq.needsARate()| freqMul.needsARate()) {
		for(int i = 0; i < block_size; i++) {
			oscillator.setFrequency(freq.getFloatValue(i)*freqMul.getFloatValue(i));
			output_buffers[0][i] = (float)oscillator.tick();
		}
	}
	else {
		oscillator.setFrequency(freq.getFloatValue()*freqMul.getFloatValue());
		for(int i = 0; i < block_size; i++) output_buffers[0][i] = (float)oscillator.tick();
	}
}

void BlitNode::reset() {
	oscillator.reset();
	oscillator.setPhase(getProperty(Lav_OSCILLATOR_PHASE).getFloatValue());
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createBlitNode(LavHandle serverHandle, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createBlitNode(server);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}