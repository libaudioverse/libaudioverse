/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/implementations/first_order_filter.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <utility>
#include <vector>

namespace libaudioverse_implementation {

class FirstOrderFilterNode: public Node {
	public:
	FirstOrderFilterNode(std::shared_ptr<Simulation> sim, int channels);
	void process() override;
	void configureLowpass(float freq);
	void configureHighpass(float freq);
	void configureAllpass(float freq);
	void recomputePoleAndZero();
	FirstOrderFilter** filters;
	int channels;
};

FirstOrderFilterNode::FirstOrderFilterNode(std::shared_ptr<Simulation> sim, int channels): Node(Lav_OBJTYPE_FIRST_ORDER_FILTER_NODE, sim, channels, channels) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	filters = new FirstOrderFilter*[channels];
	for(int i = 0; i < channels; i++) filters[i] = new FirstOrderFilter(simulation->getSr());
	this->channels = channels;
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createFirstOrderFilterNode(std::shared_ptr<Simulation> simulation, int channels) {
	auto retval = std::shared_ptr<FirstOrderFilterNode>(new FirstOrderFilterNode(simulation, channels), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void FirstOrderFilterNode::process() {
	auto &poleProp = getProperty(Lav_FIRST_ORDER_FILTER_POLE);
	auto &zeroProp = getProperty(Lav_FIRST_ORDER_FILTER_ZERO);
	//Four cases: none is a-rate, pole is a-rate, zero is a-rate, botha re a-rate.
	if(zeroProp.needsARate() && poleProp.needsARate()) {
		for(int i = 0; i < channels; i++) {
			auto &filt = *filters[i];
			for(int j = 0; j < block_size; j++) {
				filt.setZeroPosition(zeroProp.getFloatValue(j), false);
				filt.setPolePosition(poleProp.getFloatValue(j), false);
				filt.normalize();
				output_buffers[i][j] = filt.tick(input_buffers[i][j]);
			}
		}
	}
	else if(zeroProp.needsARate()) {
		for(int i = 0; i < channels; i++) {
			auto &filt = *filters[i];
			for(int j = 0; j < block_size; j++) {
				filt.setZeroPosition(zeroProp.getFloatValue(j));
				output_buffers[i][j] = filt.tick(input_buffers[i][j]);
			}
		}
	}
	else if(poleProp.needsARate()) {
		for(int i = 0; i < channels; i++) {
			auto &filt = *filters[i];
			for(int j = 0; j < block_size; j++) {
				filt.setPolePosition(poleProp.getFloatValue(j));
				output_buffers[i][j] = filt.tick(input_buffers[i][j]);
			}
		}
	}
	else {
		for(int i = 0; i < channels; i++) {
			auto &filt = *filters[i];
			filt.setPolePosition(poleProp.getFloatValue(), false);
			filt.setZeroPosition(zeroProp.getFloatValue(), false);
			filt.normalize();
			for(int j = 0; j < block_size; j++) {
				output_buffers[i][j] = filt.tick(input_buffers[i][j]);
			}
		}
	}
}

void FirstOrderFilterNode::configureLowpass(float freq) {
	for(int i = 0; i < channels; i++) filters[i]->configureLowpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::configureHighpass(float freq) {
	for(int i = 0; i < channels; i++) filters[i]->configureHighpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::configureAllpass(float freq) {
	for(int i = 0; i < channels; i++) filters[i]->configureAllpass(freq);
	recomputePoleAndZero();
}

void FirstOrderFilterNode::recomputePoleAndZero() {
	getProperty(Lav_FIRST_ORDER_FILTER_POLE).setFloatValue(filters[0]->getPolePosition());
	getProperty(Lav_FIRST_ORDER_FILTER_ZERO).setFloatValue(filters[0]->getZeroPosition());
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createFirstOrderFilterNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createFirstOrderFilterNode(simulation, channels);
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