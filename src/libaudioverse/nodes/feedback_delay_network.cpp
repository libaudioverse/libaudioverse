/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/implementations/feedback_delay_network.hpp>
#include <libaudioverse/implementations/one_pole_filter.hpp>
#include <libaudioverse/nodes/feedback_delay_network.hpp>
#include <memory>
#include <algorithm>
#include <limits> //FDN uses infinity and -infinity in the constructor.

namespace libaudioverse_implementation {

FeedbackDelayNetworkNode::FeedbackDelayNetworkNode(std::shared_ptr<Server> server, float maxDelay, int channels):
Node(Lav_OBJTYPE_FEEDBACK_DELAY_NETWORK_NODE, server, channels, channels) {
	max_delay = maxDelay;
	this->channels = channels;
	network = new FeedbackDelayNetwork<InterpolatedDelayLine>(channels, maxDelay, server->getSr());
	last_output = allocArray<float>(channels);
	next_input=allocArray<float>(channels);
	gains = allocArray<float>(channels);
	for(int i = 0; i < channels; i++) gains[i] = 1.0f;
	getProperty(Lav_FDN_MAX_DELAY).setFloatValue(maxDelay);
	for(int i= 0; i < channels; i++) {
		appendInputConnection(i, 1);
		appendOutputConnection(i, 1);
	}

	//Allocate and configure the filters.
	filters = new OnePoleFilter*[channels];
	for(int i = 0; i < channels; i++) filters[i] = new OnePoleFilter(server->getSr());
	std::vector<float> defaultHolder(channels, 0.0f);
	//Set up the properties.
	getProperty(Lav_FDN_DELAYS).setArrayLengthRange(channels, channels);
	getProperty(Lav_FDN_DELAYS).setFloatRange(0.0, maxDelay);
	getProperty(Lav_FDN_DELAYS).replaceFloatArray(channels, &defaultHolder[0]);	
	getProperty(Lav_FDN_DELAYS).setFloatArrayDefault(defaultHolder);
	
	getProperty(Lav_FDN_OUTPUT_GAINS).setArrayLengthRange(channels, channels);
	getProperty(Lav_FDN_OUTPUT_GAINS).setFloatRange(-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	defaultHolder.clear();
	defaultHolder.resize(channels, 1.0f);
	getProperty(Lav_FDN_OUTPUT_GAINS).replaceFloatArray(channels, &defaultHolder[0]);
	getProperty(Lav_FDN_OUTPUT_GAINS).setFloatArrayDefault(defaultHolder);
	//Identity matrix.
	defaultHolder.clear();
	defaultHolder.resize(channels*channels, 0.0f);
	//Build an identity matrix.
	for(int i = 0; i < channels; i++) defaultHolder[i*channels+i] = 0.0f;
	getProperty(Lav_FDN_MATRIX).setArrayLengthRange(channels*channels, channels*channels);
	getProperty(Lav_FDN_MATRIX).setFloatRange(-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	getProperty(Lav_FDN_MATRIX).replaceFloatArray(channels*channels, &defaultHolder[0]);
	getProperty(Lav_FDN_MATRIX).setFloatArrayDefault(defaultHolder);
	
	//The filters.
	getProperty(Lav_FDN_FILTER_TYPES).setArrayLengthRange(channels, channels);
	getProperty(Lav_FDN_FILTER_TYPES).zeroArray(channels);
	getProperty(Lav_FDN_FILTER_FREQUENCIES).setArrayLengthRange(channels, channels);
	getProperty(Lav_FDN_FILTER_FREQUENCIES).zeroArray(channels);
	
	setShouldZeroOutputBuffers(false);
}

FeedbackDelayNetworkNode::~FeedbackDelayNetworkNode() {
	delete network;
	freeArray(last_output);
	freeArray(next_input);
	freeArray(gains);
}

std::shared_ptr<Node> createFeedbackDelayNetworkNode(std::shared_ptr<Server> server, float maxDelay, int channels) {
	return standardNodeCreation<FeedbackDelayNetworkNode>(server, maxDelay, channels);
}

void FeedbackDelayNetworkNode::process() {
	//First, check an d set properties.
	//We just do this inline because it's trivial.
	if(werePropertiesModified(this, Lav_FDN_DELAYS)) {
		setDelays(getProperty(Lav_FDN_DELAYS).getFloatArrayPtr());
	}
	if(werePropertiesModified(this, Lav_FDN_MATRIX)) {
		setMatrix(getProperty(Lav_FDN_MATRIX).getFloatArrayPtr());
	}
	if(werePropertiesModified(this, Lav_FDN_OUTPUT_GAINS)) {
		setOutputGains(getProperty(Lav_FDN_OUTPUT_GAINS).getFloatArrayPtr());
	}
	if(werePropertiesModified(this, Lav_FDN_FILTER_TYPES, Lav_FDN_FILTER_FREQUENCIES)) {
		configureFilters(
		getProperty(Lav_FDN_FILTER_TYPES).getIntArrayPtr(),
		getProperty(Lav_FDN_FILTER_FREQUENCIES).getFloatArrayPtr());
	}
	for(int i = 0	; i < block_size; i++) {
		network->computeFrame(last_output);
		for(int j = 0; j < num_output_buffers; j++) { 	
			output_buffers[j][i] = last_output[j]*gains[j];
			next_input[j] = input_buffers[j][i];
			//Apply the filter.
			last_output[j] = filters[j]->tick(last_output[j]);
		}
		network->advance(next_input, last_output);
	}
}

void FeedbackDelayNetworkNode::setMatrix(float* values) {
	network->setMatrix(values);
}

void FeedbackDelayNetworkNode::setOutputGains(float* values) {
	std::copy(values, values+channels, gains);
}

void FeedbackDelayNetworkNode::setDelays(float* values) {
	network->setDelays(values);
}

void FeedbackDelayNetworkNode::configureFilters(int* types, float* frequencies) {
	for(int i = 0; i < channels; i++) {
		if(types[i] == Lav_FDN_FILTER_TYPE_DISABLED) {
			//Configure a passthrough filter.
			filters[i]->setCoefficients(1.0, 0.0);
		}
		else {
			filters[i]->setPoleFromFrequency(frequencies[i], types[i] == Lav_FDN_FILTER_TYPE_HIGHPASS);
		}
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createFeedbackDelayNetworkNode(LavHandle serverHandle, float maxDelay, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server =incomingObject<Server>(serverHandle);
	LOCK(*server);
	*destination = outgoingObject<Node>(createFeedbackDelayNetworkNode(
	server, maxDelay, channels));
	PUB_END
}

}