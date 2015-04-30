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

class BiquadNode: public Node {
	public:
	BiquadNode(std::shared_ptr<Simulation> sim, unsigned int channels);
	void process();
	void reconfigure();
	private:
	std::vector<IIRFilter> biquads;
	int prev_type;
};

BiquadNode::BiquadNode(std::shared_ptr<Simulation> sim, unsigned int channels): Node(Lav_OBJTYPE_BIQUAD_NODE, sim, channels, channels) {
	biquads.resize(channels);
	//configure all of them.
	prev_type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createBiquadNode(std::shared_ptr<Simulation> simulation, unsigned int channels) {
	auto retval = std::shared_ptr<BiquadNode>(new BiquadNode(simulation, channels), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void BiquadNode::reconfigure() {
	int type = getProperty(Lav_BIQUAD_FILTER_TYPE).getIntValue();
	float sr = simulation->getSr();
	float frequency = getProperty(Lav_BIQUAD_FREQUENCY).getFloatValue();
	float q = getProperty(Lav_BIQUAD_Q).getFloatValue();
	float dbgain= getProperty(Lav_BIQUAD_DBGAIN).getFloatValue();
	for(auto &i: biquads) {
		i.configureBiquad(type, sr, frequency, dbgain, q);
		if(type != prev_type) i.clearHistories();
	}
	prev_type = type;
}

void BiquadNode::process() {
	reconfigure(); //always reconfigure the biquad, so that properties are k-rate.
	//doing this this way may make the algorithm morecache- friendly on some compilers/systems.
	//It also avoids a large number of extraneous lookups in the vctor.
	for(int j = 0; j < biquads.size(); j++) {
		IIRFilter &bq = biquads[j];
		for(unsigned int i = 0; i < block_size; i++) {
			output_buffers[j][i] = bq.tick(input_buffers[j][i]);
		}
	}
}

Lav_PUBLIC_FUNCTION LavError Lav_createBiquadNode(LavHandle simulationHandle, unsigned int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createBiquadNode(simulation, channels));
	PUB_END
}

