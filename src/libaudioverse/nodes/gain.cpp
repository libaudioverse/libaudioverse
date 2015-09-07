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
#include <algorithm>

namespace libaudioverse_implementation {

class GainNode: public Node {
	public:
	GainNode(std::shared_ptr<Simulation> sim);
	void process();
};

GainNode::GainNode(std::shared_ptr<Simulation> sim): Node(Lav_OBJTYPE_GAIN_NODE, sim, 0, 0) {
}

std::shared_ptr<Node> createGainNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<GainNode>(simulation);
}

void GainNode::process() {
	for(unsigned int i = 0; i < input_buffers.size(); i++) {
		std::copy(input_buffers[i], input_buffers[i]+block_size, output_buffers[i]);
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createGainNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createGainNode(simulation);
	retval->resize(channels, channels);
	retval->appendInputConnection(0, channels);
	retval->appendOutputConnection(0, channels);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

}