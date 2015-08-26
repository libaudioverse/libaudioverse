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
#include <libaudioverse/implementations/allpass.hpp>
#include <libaudioverse/implementations/delayline.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

class AllpassNode: public Node {
	public:
	AllpassNode(std::shared_ptr<Simulation> sim, int channels, int maxDelay);
	~AllpassNode();
	void reconfigureCoefficient();
	void reconfigureDelay();
	void reconfigureInterpolationTime();
	void process();
	int channels;
	AllpassFilter<CrossfadingDelayLine> **allpasses;
};

AllpassNode::AllpassNode(std::shared_ptr<Simulation> sim, int channels, int maxDelay): Node(Lav_OBJTYPE_ALLPASS_NODE, sim, channels, channels) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	if(maxDelay < 1) ERROR(Lav_ERROR_RANGE, "You need to allow for at least 1 sample of delay.");
	getProperty(Lav_ALLPASS_DELAY_SAMPLES_MAX).setIntValue(maxDelay);
	getProperty(Lav_ALLPASS_DELAY_SAMPLES).setIntRange(0, maxDelay);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	allpasses = new AllpassFilter<CrossfadingDelayLine>*[channels];
	//We add 1 here to make sure we don't run up against floating point accuracy.
	for(int i = 0; i < channels; i++) allpasses[i] = new AllpassFilter<CrossfadingDelayLine>((maxDelay+1)/simulation->getSr(), simulation->getSr());
	this->channels = channels;
}

std::shared_ptr<Node> createAllpassNode(std::shared_ptr<Simulation> simulation, int channels, int maxDelay) {
	auto retval = std::shared_ptr<AllpassNode>(new AllpassNode(simulation, channels, maxDelay), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

AllpassNode::~AllpassNode() {
	for(int i = 0; i < channels; i++) delete allpasses[i];
	delete[] allpasses;
}

void AllpassNode::reconfigureCoefficient() {
	float c = getProperty(Lav_ALLPASS_COEFFICIENT).getFloatValue();
	for(int i = 0; i < channels; i++) allpasses[i]->setCoefficient(c);
}

void AllpassNode::reconfigureDelay() {
	int d = getProperty(Lav_ALLPASS_DELAY_SAMPLES).getIntValue();
	for(int i = 0; i < channels; i++) allpasses[i]->line.setDelayInSamples(d);
}

void AllpassNode::reconfigureInterpolationTime() {
	float it = getProperty(Lav_ALLPASS_INTERPOLATION_TIME).getFloatValue();
	for(int i = 0; i < channels; i++) allpasses[i]->line.setInterpolationTime(it);
}

void AllpassNode::process() {
	if(werePropertiesModified(this, Lav_ALLPASS_INTERPOLATION_TIME)) reconfigureInterpolationTime();
	if(werePropertiesModified(this, Lav_ALLPASS_COEFFICIENT)) reconfigureCoefficient();
	if(werePropertiesModified(this, Lav_ALLPASS_DELAY_SAMPLES)) reconfigureDelay();
	for(int i = 0; i < channels; i++) {
		auto &f = *allpasses[i];
		for(int j = 0; j < block_size; j++) output_buffers[i][j] = f.tick(input_buffers[i][j]);
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createAllpassNode(LavHandle simulationHandle, int channels, int maxDelay, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createAllpassNode(simulation, channels, maxDelay);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

}