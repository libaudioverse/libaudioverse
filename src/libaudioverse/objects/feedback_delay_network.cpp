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
#include <libaudioverse/implementations/feedback_delay_network.hpp>
#include <libaudioverse/objects/feedback_delay_network.hpp>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
#include <math.h>

LavFeedbackDelayNetworkObject::LavFeedbackDelayNetworkObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, int lines):
LavObject(Lav_OBJTYPE_FEEDBACK_DELAY_NETWORK, simulation, lines, lines) {
	max_delay = maxDelay;
	line_count = lines;
	network = new LavFeedbackDelayNetwork(lines, maxDelay, simulation->getSr());
	lastOutput = new float[lines]();
	nextInput = new float[lines]();
	gains = new float[lines]();
	for(int i = 0; i < lines; i++) gains[i] = 1.0f;
	getProperty(Lav_FDN_MAX_DELAY).setFloatValue(maxDelay);
}

LavFeedbackDelayNetworkObject::~LavFeedbackDelayNetworkObject() {
	delete network;
	delete[] lastOutput;
	delete[] nextInput;
	delete[] gains;
}

std::shared_ptr<LavObject> createFeedbackDelayNetworkObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, int lines) {
	return std::make_shared<LavFeedbackDelayNetworkObject>(simulation, maxDelay, lines);
}

void LavFeedbackDelayNetworkObject::process() {
	for(int i = 0; i < block_size; i++) {
		network->computeFrame(lastOutput);
		for(unsigned int j = 0; j < getOutputCount(); j++) {
			nextInput[j] = inputs[j][i];
			outputs[j][i] = lastOutput[j]*gains[j];
		}
		network->advance(nextInput, lastOutput);
	}
}

void LavFeedbackDelayNetworkObject::setFeedbackMatrix(int length, float* values) {
	if(length != line_count*line_count) throw LavErrorException(Lav_ERROR_RANGE);
	network->setFeedbackMatrix(values);
}

void LavFeedbackDelayNetworkObject::setOutputGains(int count, float* values) {
	if(count != line_count) throw LavErrorException(Lav_ERROR_RANGE);
	std::copy(values, values+count, gains);
}

void LavFeedbackDelayNetworkObject::setDelays(int length, float* values) {
	if(length != line_count) throw LavErrorException(Lav_ERROR_RANGE);
	network->setDelays(values);
}

void LavFeedbackDelayNetworkObject::setFeedbackDelayMatrix(int length, float* values) {
	if(length != line_count*line_count) throw LavErrorException(Lav_ERROR_RANGE);
	network->setFeedbackDelayMatrix(values);
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createFeedbackDelayNetworkObject(LavSimulation* sim, float maxDelay, int lines, LavObject** destination) {
	PUB_BEGIN
	LOCK(*sim);
	*destination = outgoingPointer<LavObject>(createFeedbackDelayNetworkObject(
incomingPointer<LavSimulation>(sim), maxDelay, lines));
	PUB_END
}

#define FDN_OR_ERROR if(obj->getType() != Lav_OBJTYPE_FEEDBACK_DELAY_NETWORK) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);

Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkObjectSetFeedbackMatrix(LavObject* obj, int count, float* values) {
	PUB_BEGIN
	LOCK(*obj);
	FDN_OR_ERROR
	((LavFeedbackDelayNetworkObject*)obj)->setFeedbackMatrix(count, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkObjectSetOutputGains(LavObject* obj, int count, float* values) {
	PUB_BEGIN
	LOCK(*obj);
	FDN_OR_ERROR
	((LavFeedbackDelayNetworkObject*)obj)->setOutputGains(count, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkObjectSetDelays(LavObject* obj, int count, float* values) {
	PUB_BEGIN
	LOCK(*obj);
	FDN_OR_ERROR
	((LavFeedbackDelayNetworkObject*)obj)->setDelays(count, values);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_feedbackDelayNetworkObjectSetFeedbackDelayMatrix(LavObject* obj, int count, float* values) {
	PUB_BEGIN
	LOCK(*obj);
	FDN_OR_ERROR
	((LavFeedbackDelayNetworkObject*)obj)->setFeedbackDelayMatrix(count, values);
	PUB_END
}
