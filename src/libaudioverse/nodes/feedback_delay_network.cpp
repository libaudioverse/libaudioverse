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
#include <libaudioverse/implementations/feedback_delay_network.hpp>
#include <libaudioverse/nodes/feedback_delay_network.hpp>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
#include <math.h>

namespace libaudioverse_implementation {

FeedbackDelayNetworkNode::FeedbackDelayNetworkNode(std::shared_ptr<Simulation> simulation, float maxDelay, int lines):
Node(Lav_OBJTYPE_FEEDBACK_DELAY_NETWORK_NODE, simulation, lines, lines) {
	max_delay = maxDelay;
	line_count = lines;
	network = new FeedbackDelayNetwork(lines, maxDelay, simulation->getSr());
	lastOutput = allocArray<float>(lines);
	nextInput = allocArray<float>(lines);
	gains = allocArray<float>(lines);
	for(int i = 0; i < lines; i++) gains[i] = 1.0f;
	getProperty(Lav_FDN_MAX_DELAY).setFloatValue(maxDelay);
	appendInputConnection(0, lines);
	appendOutputConnection(0, lines);
}

FeedbackDelayNetworkNode::~FeedbackDelayNetworkNode() {
	delete network;
	freeArray(lastOutput);
	freeArray(nextInput);
	freeArray(gains);
}

std::shared_ptr<Node> createFeedbackDelayNetworkNode(std::shared_ptr<Simulation> simulation, float maxDelay, int lines) {
	auto retval = std::shared_ptr<FeedbackDelayNetworkNode>(new FeedbackDelayNetworkNode(simulation, maxDelay, lines), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void FeedbackDelayNetworkNode::process() {
	for(int i = 0; i < block_size; i++) {
		network->computeFrame(lastOutput);
		for(int j = 0; j < num_output_buffers; j++) {
			nextInput[j] = input_buffers[j][i];
			output_buffers[j][i] = lastOutput[j]*gains[j];
		}
		network->advance(nextInput, lastOutput);
	}
}

void FeedbackDelayNetworkNode::setFeedbackMatrix(int length, float* values) {
	if(length != line_count*line_count) throw LavErrorException(Lav_ERROR_RANGE);
	network->setFeedbackMatrix(values);
}

void FeedbackDelayNetworkNode::setOutputGains(int count, float* values) {
	if(count != line_count) throw LavErrorException(Lav_ERROR_RANGE);
	std::copy(values, values+count, gains);
}

void FeedbackDelayNetworkNode::setDelays(int length, float* values) {
	if(length != line_count) throw LavErrorException(Lav_ERROR_RANGE);
	network->setDelays(values);
}

void FeedbackDelayNetworkNode::setFeedbackDelayMatrix(int length, float* values) {
	if(length != line_count*line_count) throw LavErrorException(Lav_ERROR_RANGE);
	network->setFeedbackDelayMatrix(values);
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createFeedbackDelayNetworkNode(LavHandle simulationHandle, float maxDelay, int lines, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createFeedbackDelayNetworkNode(
	simulation, maxDelay, lines));
	PUB_END
}

#define FDN_OR_ERROR if(node->getType() != Lav_OBJTYPE_FEEDBACK_DELAY_NETWORK_NODE) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);

Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkNodeSetFeedbackMatrix(LavHandle nodeHandle, int count, float* values) {
	PUB_BEGIN
	auto node=incomingObject<Node>(nodeHandle);
	LOCK(*node);
	FDN_OR_ERROR
	auto fdn=std::static_pointer_cast<FeedbackDelayNetworkNode>(node);
	fdn->setFeedbackMatrix(count, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkNodeSetOutputGains(LavHandle nodeHandle, int count, float* values) {
	PUB_BEGIN
	auto node=incomingObject<Node>(nodeHandle);
	LOCK(*node);
	FDN_OR_ERROR
	auto fdn = std::static_pointer_cast<FeedbackDelayNetworkNode>(node);
	fdn->setOutputGains(count, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkNodeSetDelays(LavHandle nodeHandle, int count, float* values) {
	PUB_BEGIN
	auto node= incomingObject<Node>(nodeHandle);
	LOCK(*node);
	FDN_OR_ERROR
	auto fdn=std::static_pointer_cast<FeedbackDelayNetworkNode>(node);
	fdn->setDelays(count, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkNodeSetFeedbackDelayMatrix(LavHandle nodeHandle, int count, float* values) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	FDN_OR_ERROR
	auto fdn=std::static_pointer_cast<FeedbackDelayNetworkNode>(node);
	fdn->setFeedbackDelayMatrix(count, values);
	PUB_END
}

}