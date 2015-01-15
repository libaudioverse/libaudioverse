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
#include <libaudioverse/private/kernels.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <utility>
#include <vector>
#include <libaudioverse/private/iir.hpp>

class LavBiquadNode: public LavNode {
	public:
	LavBiquadNode(std::shared_ptr<LavSimulation> sim, unsigned int channels);
	void process();
	void reconfigure();
	private:
	std::vector<LavIIRFilter> biquads;
	int prev_type;
};

LavBiquadNode::LavBiquadNode(std::shared_ptr<LavSimulation> sim, unsigned int channels): LavNode(Lav_NODETYPE_BIQUAD, sim, channels, channels) {
	biquads.resize(channels);
	//configure all of them.
	prev_type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
	reconfigure();
	//we can grab the properties dictionary ourselves, if we want.
	for(auto i = properties.begin(); i != properties.end(); i++) {
		if(i->first < STANDARD_PROPERTIES_BEGIN) continue; //this is a standard property.
		properties[i->first].setPostChangedCallback([this]() {reconfigure();});
	}
}

std::shared_ptr<LavNode> createBiquadNode(std::shared_ptr<LavSimulation> sim, unsigned int channels) {
	auto retval = std::shared_ptr<LavBiquadNode>(new LavBiquadNode(sim, channels), LavNodeDeleter);
	sim->associateNode(retval);
	return retval;
}

void LavBiquadNode::reconfigure() {
	int type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
	float sr = simulation->getSr();
	float frequency = getProperty(Lav_BIQUAD_FREQUENCY).getFloatValue();
	float q = getProperty(Lav_BIQUAD_Q).getFloatValue();
	float dbgain= getProperty(Lav_BIQUAD_DBGAIN).getFloatValue();
	for(auto &i: biquads) {
		if(type != prev_type) i.clearHistories();
		i.configureBiquad(type, sr, frequency, dbgain, q);
	}
	prev_type = type;
}

void LavBiquadNode::process() {
	//doing this this way may make the algorithm morecache- friendly on some compilers/systems.
	//It also avoids a large number of extraneous lookups in the vctor.
	for(int j = 0; j < biquads.size(); j++) {
		LavIIRFilter &bq = biquads[j];
		for(unsigned int i = 0; i < block_size; i++) {
			outputs[j][i] = bq.tick(inputs[j][i]);
		}
	}
}

Lav_PUBLIC_FUNCTION LavError Lav_createBiquadObject(LavSimulation* sim, unsigned int channels, LavNode** destination) {
	PUB_BEGIN
	LOCK(*sim);
	*destination = outgoingPointer<LavNode>(createBiquadNode(incomingPointer<LavSimulation>(sim), channels));
	PUB_END
}

