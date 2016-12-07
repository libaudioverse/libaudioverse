/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/first_order_filter.hpp>
#include <libaudioverse/implementations/first_order_filter.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/multichannel_filter_bank.hpp>
#include <memory>

namespace libaudioverse_implementation {

FirstOrderFilterNode::FirstOrderFilterNode(std::shared_ptr<Server> s, int channels): Node(Lav_OBJTYPE_FIRST_ORDER_FILTER_NODE, s, channels, channels),
bank(server->getSr()) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	bank.setChannelCount(channels);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createFirstOrderFilterNode(std::shared_ptr<Server> server, int channels) {
	return standardNodeCreation<FirstOrderFilterNode>(server, channels);
}

void FirstOrderFilterNode::process() {
	auto &poleProp = getProperty(Lav_FIRST_ORDER_FILTER_POLE);
	auto &zeroProp = getProperty(Lav_FIRST_ORDER_FILTER_ZERO);
	//Four cases: none is a-rate, pole is a-rate, zero is a-rate, both are a-rate.
	if(zeroProp.needsARate() && poleProp.needsARate()) {
		bank.process(block_size, &input_buffers[0], &output_buffers[0], [&] (FirstOrderFilter& filt, int index) {
			filt.setZeroPosition(zeroProp.getFloatValue(index), false);
			filt.setPolePosition(poleProp.getFloatValue(index), false);
			filt.normalize();
		});
	}
	else if(zeroProp.needsARate()) {
		bank.process(block_size, &input_buffers[0], &output_buffers[0], [&] (FirstOrderFilter& filt, int index) {
			filt.setZeroPosition(zeroProp.getFloatValue(index), false);
			filt.normalize();
		});
	}
	else if(poleProp.needsARate()) {
		bank.process(block_size, &input_buffers[0], &output_buffers[0], [&] (FirstOrderFilter& filt, int index) {
			filt.setPolePosition(poleProp.getFloatValue(index), false);
			filt.normalize();
		});
	}
	else {
		bank->setZeroPosition(zeroProp.getFloatValue(), false);
		bank->setPolePosition(poleProp.getFloatValue(), false);
		bank->normalize();
		bank.process(block_size, &input_buffers[0], &output_buffers[0]);
	}
}

void FirstOrderFilterNode::configureLowpass(float freq) {
	bank->configureLowpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::configureHighpass(float freq) {
	bank->configureHighpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::configureAllpass(float freq) {
	bank->configureAllpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::recomputePoleAndZero() {
	auto& filt = *bank;
	getProperty(Lav_FIRST_ORDER_FILTER_POLE).setFloatValue(filt.getPolePosition());
	getProperty(Lav_FIRST_ORDER_FILTER_ZERO).setFloatValue(filt.getZeroPosition());
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createFirstOrderFilterNode(LavHandle serverHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval= createFirstOrderFilterNode(server, channels);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

//I'm not typing this 3 times.
#define conf(which) \
Lav_PUBLIC_FUNCTION LavError Lav_firstOrderFilterNodeConfigure##which(LavHandle nodeHandle, float frequency) {\
	PUB_BEGIN\
	auto n = incomingObject<FirstOrderFilterNode>(nodeHandle);\
	LOCK(*n);\
	n->configure##which(frequency);\
	PUB_END\
}

conf(Lowpass)
conf(Highpass)
conf(Allpass)

}