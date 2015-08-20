/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/implementations/one_pole_filter.hpp>
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

class OnePoleFilterNode: public Node {
	public:
	OnePoleFilterNode(std::shared_ptr<Simulation> sim, int channels);
	void process() override;
	void reconfigureFilters();
	OnePoleFilter** filters;
	int channels;
};

OnePoleFilterNode::OnePoleFilterNode(std::shared_ptr<Simulation> sim, int channels): Node(Lav_OBJTYPE_ONE_POLE_FILTER_NODE, sim, channels, channels) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	filters = new OnePoleFilter*[channels];
	for(int i = 0; i < channels; i++) filters[i] = new OnePoleFilter(simulation->getSr());
	getProperty(Lav_ONE_POLE_FILTER_FREQUENCY).setFloatRange(0, simulation->getSr()/2.0);
	this->channels = channels;
	reconfigureFilters();
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createOnePoleFilterNode(std::shared_ptr<Simulation> simulation, int channels) {
	auto retval = std::shared_ptr<OnePoleFilterNode>(new OnePoleFilterNode(simulation, channels), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void OnePoleFilterNode::reconfigureFilters() {
	float freq = getProperty(Lav_ONE_POLE_FILTER_FREQUENCY).getFloatValue();
	bool isHighpass = getProperty(Lav_ONE_POLE_FILTER_IS_HIGHPASS).getIntValue() == 1; //==1 prevents a performance warning from VC++.
	for(int i = 0; i < channels; i++) filters[i]->setPoleFromFrequency(freq, isHighpass);
}

void OnePoleFilterNode::process() {
	if(werePropertiesModified(this, Lav_ONE_POLE_FILTER_IS_HIGHPASS, Lav_ONE_POLE_FILTER_FREQUENCY)) reconfigureFilters();
	for(int i = 0; i < channels; i++) {
		auto &filt = *filters[i];
		for(int j = 0; j < block_size; j++) output_buffers[i][j] = filt.tick(input_buffers[i][j]);
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createOnePoleFilterNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createOnePoleFilterNode(simulation, channels);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

}