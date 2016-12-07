/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/allpass.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/multichannel_filter_bank.hpp>
#include <libaudioverse/implementations/allpass.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

AllpassNode::AllpassNode(std::shared_ptr<Server> s, int channels, int maxDelay): Node(Lav_OBJTYPE_ALLPASS_NODE, s, channels, channels),
//The +1 here deals with any floating point inaccuracies.
bank((maxDelay+1)/server->getSr(), server->getSr()) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	if(maxDelay < 1) ERROR(Lav_ERROR_RANGE, "You need to allow for at least 1 sample of delay.");
	getProperty(Lav_ALLPASS_DELAY_SAMPLES_MAX).setIntValue(maxDelay);
	getProperty(Lav_ALLPASS_DELAY_SAMPLES).setIntRange(0, maxDelay);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	bank.setChannelCount(channels);
}

std::shared_ptr<Node> createAllpassNode(std::shared_ptr<Server> server, int channels, int maxDelay) {
	return standardNodeCreation<AllpassNode>(server, channels, maxDelay);
}

void AllpassNode::reconfigureCoefficient() {
	float c = getProperty(Lav_ALLPASS_COEFFICIENT).getFloatValue();
	bank->setCoefficient(c);
}

void AllpassNode::reconfigureDelay() {
	int d = getProperty(Lav_ALLPASS_DELAY_SAMPLES).getIntValue();
	bank->line.setDelayInSamples(d);
}

void AllpassNode::reconfigureInterpolationTime() {
	float it = getProperty(Lav_ALLPASS_INTERPOLATION_TIME).getFloatValue();
	bank->line.setInterpolationTime(it);
}

void AllpassNode::process() {
	if(werePropertiesModified(this, Lav_ALLPASS_INTERPOLATION_TIME)) reconfigureInterpolationTime();
	if(werePropertiesModified(this, Lav_ALLPASS_COEFFICIENT)) reconfigureCoefficient();
	if(werePropertiesModified(this, Lav_ALLPASS_DELAY_SAMPLES)) reconfigureDelay();
	bank.process(block_size, &input_buffers[0], &output_buffers[0]);
}

void AllpassNode::reset() {
	bank.reset();
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createAllpassNode(LavHandle serverHandle, int channels, int maxDelay, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval= createAllpassNode(server, channels, maxDelay);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

}