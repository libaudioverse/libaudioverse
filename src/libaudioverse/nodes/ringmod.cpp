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
#include <memory>

namespace libaudioverse_implementation {

class RingmodNode: public Node {
	public:
	RingmodNode(std::shared_ptr<Simulation> sim);
	void process();
};

RingmodNode::RingmodNode(std::shared_ptr<Simulation> sim): Node(Lav_OBJTYPE_RINGMOD_NODE, sim, 2, 1) {
	appendInputConnection(0, 1);
	appendInputConnection(1, 1);
	appendOutputConnection(0, 1);
}

std::shared_ptr<Node> createRingmodNode(std::shared_ptr<Simulation> simulation) {
	return standardNodeCreation<RingmodNode>(simulation);
}

void RingmodNode::process() {
	multiplicationKernel(block_size, input_buffers[0], input_buffers[1], output_buffers[0]);
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createRingmodNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject<Node>(createRingmodNode(simulation));
	PUB_END
}

}