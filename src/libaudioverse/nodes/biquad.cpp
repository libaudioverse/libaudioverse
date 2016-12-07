/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/biquad.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/biquad.hpp>
#include <libaudioverse/private/multichannel_filter_bank.hpp>
#include <memory>


namespace libaudioverse_implementation {

BiquadNode::BiquadNode(std::shared_ptr<Server> s, unsigned int channels): Node(Lav_OBJTYPE_BIQUAD_NODE, s, channels, channels),
bank(server->getSr()) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	bank.setChannelCount(channels);
	prev_type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createBiquadNode(std::shared_ptr<Server> server, unsigned int channels) {
	return standardNodeCreation<BiquadNode>(server, channels);
}

void BiquadNode::reconfigure() {
	int type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
	float sr = server->getSr();
	float frequency = getProperty(Lav_BIQUAD_FREQUENCY).getFloatValue();
	float q = getProperty(Lav_BIQUAD_Q).getFloatValue();
	float dbgain= getProperty(Lav_BIQUAD_DBGAIN).getFloatValue();
	bank->configure(type, frequency, dbgain, q);
	if(type != prev_type) bank.reset();
	prev_type = type;
}

void BiquadNode::process() {
	if(werePropertiesModified(this, Lav_BIQUAD_FILTER_TYPE, Lav_BIQUAD_DBGAIN, Lav_BIQUAD_FREQUENCY, Lav_BIQUAD_Q)) reconfigure();
	bank.process(block_size, &input_buffers[0], &output_buffers[0]);
}

void BiquadNode::reset() {
	bank.reset();
}

Lav_PUBLIC_FUNCTION LavError Lav_createBiquadNode(LavHandle serverHandle, unsigned int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server =incomingObject<Server>(serverHandle);
	LOCK(*server);
	*destination = outgoingObject<Node>(createBiquadNode(server, channels));
	PUB_END
}

}