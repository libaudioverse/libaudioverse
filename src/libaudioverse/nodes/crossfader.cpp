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
#include <libaudioverse/nodes/crossfader.hpp>
#include <memory>

namespace libaudioverse_implementation {

CrossfaderNode::CrossfaderNode(std::shared_ptr<Simulation> sim, int channels, int inputs): Node(Lav_OBJTYPE_CROSSFADER_NODE, sim, channels*inputs, channels)  {
	if(inputs < 1) ERROR(Lav_ERROR_RANGE, "It doesn't make sense to have a crossfader with less than 2 inputs.");
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "It doesn't make sense to have a crossfader with no channels.");
	for(int i = 0; i < inputs; i++) {
		appendInputConnection(i*channels, channels);
	}
	appendOutputConnection(0, channels);
	this->channels = channels;
	getProperty(Lav_CROSSFADER_CURRENT_INPUT).setIntRange(0, inputs);
	getProperty(Lav_CROSSFADER_TARGET_INPUT).setIntRange(0, inputs);
	getProperty(Lav_CROSSFADER_CURRENT_INPUT).setPostChangedCallback([&] () {
		crossfade(0.0, getProperty(Lav_CROSSFADER_CURRENT_INPUT).getIntValue());
	});
}

std::shared_ptr<Node> createCrossfaderNode(std::shared_ptr<Simulation> simulation, int channels, int inputs) {
	return standardNodeCreation<CrossfaderNode>(simulation, channels, inputs);
}

void CrossfaderNode::crossfade(float duration, int input) {
	if(input < 0 || input > getInputConnectionCount()) ERROR(Lav_ERROR_RANGE, "Can't crossfade to an input that doesn't exist.");
	if(crossfading) finishCrossfade();
	if(duration == 0.0f) {
		//Instantaneous crossfade.
		current = input;
		getProperty(Lav_CROSSFADER_CURRENT_INPUT).setIntValue(current, true);
		return;
	}
	getProperty(Lav_CROSSFADER_IS_CROSSFADING).setIntValue(1);
	getProperty(Lav_CROSSFADER_TARGET_INPUT).setIntValue(input);
	target = input;
	delta = 1.0/(simulation->getSr()*duration);
	crossfading = true;
}

void CrossfaderNode::finishCrossfade() {
	current = target;
	getProperty(Lav_CROSSFADER_CURRENT_INPUT).setIntValue(current, true);
	getProperty(Lav_CROSSFADER_IS_CROSSFADING).setIntValue(0);
	current_weight = 1.0f;
	target_weight = 0.0f;
	crossfading = false;
	simulation->enqueueTask([=] () {(*finished_callback)();});
}

void CrossfaderNode::process() {
	int i = 0;
	while(i < block_size && crossfading) {
		for(int chan = 0; chan < channels; chan ++) {
			float cs = input_buffers[current*channels+chan][i];
			float ts = input_buffers[target*channels+chan][i];
			output_buffers[chan][i] = cs*current_weight+ts*target_weight;
		}
		current_weight -= delta;
		target_weight += delta;
		i++;
		if(current_weight <= 0.0f || target_weight >= 1.0f) finishCrossfade();
	}
	//For the rest, we just copy.
	for(int chan = 0; chan < channels; chan++) {
		std::copy(input_buffers[current*channels+chan]+i, input_buffers[current*channels+chan]+block_size, output_buffers[chan]+i);
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createCrossfaderNode(LavHandle simulationHandle, int channels, int inputs, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createCrossfaderNode(simulation, channels, inputs);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_crossfaderNodeCrossfade(LavHandle nodeHandle, float duration, int input) {
	PUB_BEGIN
	auto n = incomingObject<CrossfaderNode>(nodeHandle);
	LOCK(*n);
	n->crossfade(duration, input);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_crossfaderNodeSetFinishedCallback(LavHandle nodeHandle, LavParameterlessCallback callback, void* userdata) {
	PUB_BEGIN
	auto n = incomingObject<CrossfaderNode>(nodeHandle);
	if(callback) {
		n->finished_callback->setCallback(wrapParameterlessCallback(n, callback, userdata));
	}
	else n->finished_callback->clear();
	PUB_END
}

}