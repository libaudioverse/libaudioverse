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
#include <memory>

namespace libaudioverse_implementation {

class DoppleringDelayNode: public Node {
	public:
	DoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, int channels);
	~DoppleringDelayNode();
	void process();
	protected:
	void delayChanged();
	void recomputeDelta();
	//Callbacks for when delay_samples and delay get changed.
	void updateDelay();
	void updateDelaySamples();
	//This flag prevents the above two callbacks from ping-ponging.
	bool is_syncing_properties = false;
	//Set to either Lav_DELAY_DELAY or Lav_DELAY_DELAY_SAMPLES if the property changed. Otherwise 0.
	int last_updated_delay_property = 0;
	//Standard stuff for delay lines.
	unsigned int delay_line_length = 0;
	DoppleringDelayLine **lines;
	int channels;
};

DoppleringDelayNode::DoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, int channels): Node(Lav_OBJTYPE_DOPPLERING_DELAY_NODE, simulation, channels, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "lineCOunt must be greater than 0.");
	this->channels = channels;
	lines = new DoppleringDelayLine*[channels]();
	for(unsigned int i = 0; i < channels; i++) lines[i] = new DoppleringDelayLine(maxDelay, simulation->getSr());
	getProperty(Lav_DELAY_DELAY).setFloatRange(0.0f, maxDelay);
	getProperty(Lav_DELAY_DELAY_SAMPLES).setDoubleRange(0.0, maxDelay*(double)simulation->getSr());
	//These callbacks link these properties to each other.
	getProperty(Lav_DELAY_DELAY).setPostChangedCallback([&] () {updateDelaySamples();});
	getProperty(Lav_DELAY_DELAY_SAMPLES).setPostChangedCallback([&] () {updateDelay();});
	//finally, set the read-only max delay.
	getProperty(Lav_DELAY_DELAY_MAX).setFloatValue(maxDelay);
	//Set us up as though delay just changed.
	last_updated_delay_property = Lav_DELAY_DELAY;
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createDoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, int channels) {
	return standardNodeCreation<DoppleringDelayNode>(simulation, maxDelay, channels);
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
	if(last_updated_delay_property == Lav_DELAY_DELAY) {
		float newDelay = getProperty(Lav_DELAY_DELAY).getFloatValue();
		for(int i = 0; i < channels; i++) lines[i]->setDelay(newDelay);
	}
	else {
		int delayInSamples = (int)getProperty(Lav_DELAY_DELAY_SAMPLES).getDoubleValue();
		for(int i = 0; i < channels; i++) lines[i]->setDelayInSamples(delayInSamples);
	}
	last_updated_delay_property = 0;
}

void DoppleringDelayNode::updateDelaySamples() {
	if(is_syncing_properties) return; //The other callback is why we were called.
	double newValue = getProperty(Lav_DELAY_DELAY).getFloatValue()*(double)simulation->getSr();
	is_syncing_properties = true;
	getProperty(Lav_DELAY_DELAY_SAMPLES).setDoubleValue(newValue);
	is_syncing_properties = false;
	last_updated_delay_property = Lav_DELAY_DELAY;
}

void DoppleringDelayNode::updateDelay() {
	if(is_syncing_properties) return; //The other callback is why we were called.
	double newValue = getProperty(Lav_DELAY_DELAY_SAMPLES).getDoubleValue()/simulation->getSr();
	is_syncing_properties = true;
	getProperty(Lav_DELAY_DELAY).setFloatValue(newValue);
	is_syncing_properties = false;
	last_updated_delay_property = Lav_DELAY_DELAY_SAMPLES;
}

void DoppleringDelayNode::process() {
	if(last_updated_delay_property != 0) delayChanged();
	if(werePropertiesModified(this, Lav_DELAY_INTERPOLATION_TIME)) recomputeDelta();
	for(int output = 0; output < num_output_buffers; output++) {
		auto &line = *lines[output];
		for(int i = 0; i < block_size; i++) output_buffers[output][i] = line.tick(input_buffers[output][i]);
	}
}

//begin public api
Lav_PUBLIC_FUNCTION LavError Lav_createDoppleringDelayNode(LavHandle simulationHandle, float maxDelay, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto d = createDoppleringDelayNode(simulation, maxDelay, channels);
	*destination = outgoingObject(d);
	PUB_END
}

}