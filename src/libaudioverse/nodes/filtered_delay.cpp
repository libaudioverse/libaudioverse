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
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/implementations/biquad.hpp>
#include <memory>
#include <algorithm>

namespace libaudioverse_implementation {

class FilteredDelayNode: public Node {
	public:
	FilteredDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int channels);
	~FilteredDelayNode();
	void process();
	void reset() override;
	protected:
	void delayChanged();
	void recomputeDelta();
	void reconfigureBiquads();
	unsigned int delay_line_length = 0;
	CrossfadingDelayLine **lines;
	BiquadFilter** biquads;
	int prev_type; //used for biquad history resets.
	int channels;
};

FilteredDelayNode::FilteredDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int channels): Node(Lav_OBJTYPE_FILTERED_DELAY_NODE, simulation, channels, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	this->channels = channels;
	lines = new CrossfadingDelayLine*[channels];
	for(unsigned int i = 0; i < channels; i++) lines[i] = new CrossfadingDelayLine(maxDelay, simulation->getSr());
	getProperty(Lav_FILTERED_DELAY_DELAY).setFloatRange(0.0f, maxDelay);
	//finally, set the read-only max delay.
	getProperty(Lav_FILTERED_DELAY_DELAY_MAX).setFloatValue(maxDelay);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	//This node was made by merging delay and biquad, everything below here is biquad:
	biquads = new BiquadFilter*[channels]();
	for(int i= 0; i < channels; i++) biquads[i] = new BiquadFilter(simulation->getSr());
	prev_type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
}

std::shared_ptr<Node> createFilteredDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int channels) {
	return standardNodeCreation<FilteredDelayNode>(simulation, maxDelay, channels);
}

FilteredDelayNode::~FilteredDelayNode() {
	for(int i=0; i < channels; i++) delete lines[i];
	delete[] lines;
	//Biquad destructor:
	for(int i=0; i < channels; i++) delete biquads[i];
	delete[] biquads;
}

void FilteredDelayNode::recomputeDelta() {
	float time = getProperty(Lav_FILTERED_DELAY_INTERPOLATION_TIME).getFloatValue();
	for(int i = 0; i < channels; i++) lines[i]->setInterpolationTime(time);
}

void FilteredDelayNode::delayChanged() {
	float newDelay = getProperty(Lav_FILTERED_DELAY_DELAY).getFloatValue();
	for(int i= 0; i < channels; i++) lines[i]->setDelay(newDelay);
}

//Modified from the biquad node.
void FilteredDelayNode::reconfigureBiquads() {
	int type = getProperty(Lav_FILTERED_DELAY_FILTER_TYPE).getIntValue();
	float sr = simulation->getSr();
	float frequency = getProperty(Lav_FILTERED_DELAY_FREQUENCY).getFloatValue();
	float q = getProperty(Lav_FILTERED_DELAY_Q).getFloatValue();
	float dbgain= getProperty(Lav_FILTERED_DELAY_DBGAIN).getFloatValue();
	for(int i=0; i < channels; i++) {
		biquads[i]->configure(type, frequency, dbgain, q);
		if(type != prev_type) biquads[i]->reset();
	}
	prev_type = type;
}

void FilteredDelayNode::process() {
	if(werePropertiesModified(this, Lav_FILTERED_DELAY_DELAY)) delayChanged();
	if(werePropertiesModified(this, Lav_FILTERED_DELAY_INTERPOLATION_TIME)) recomputeDelta();
	reconfigureBiquads();
	float feedback = getProperty(Lav_FILTERED_DELAY_FEEDBACK).getFloatValue();
	//optimize the common case of not having feedback.
	//the only difference between these blocks is in the advance line.
	if(feedback == 0.0f) {
		for(unsigned int output = 0; output < num_output_buffers; output++) {
			auto &line = *lines[output];
			auto &filter=*biquads[output];
			line.processBuffer(block_size, input_buffers[output], output_buffers[output]);
			for(int i=0; i < block_size; i++) output_buffers[output][i] = filter.tick(output_buffers[output][i]);
		}
	}
	else {
		for(unsigned int output = 0; output < num_output_buffers; output++) {
			auto &line = *lines[output];
			auto &filter = *biquads[output];
			for(unsigned int i = 0; i < block_size; i++) {
				output_buffers[output][i] = line.computeSample();
				output_buffers[output][i] = filter.tick(output_buffers[output][i]);
				line.advance(input_buffers[output][i]+output_buffers[output][i]*feedback);
			}
		}
	}
}

void FilteredDelayNode::reset() {
	for(int i = 0; i < channels; i++) {
		biquads[i]->reset();
		lines[i]->reset();
	}
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createFilteredDelayNode(LavHandle simulationHandle, float maxDelay, unsigned int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto d = createFilteredDelayNode(simulation, maxDelay, channels);
	*destination = outgoingObject(d);
	PUB_END
}

}