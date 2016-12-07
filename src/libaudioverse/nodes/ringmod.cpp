/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/ringmod.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <memory>

namespace libaudioverse_implementation {

RingmodNode::RingmodNode(std::shared_ptr<Server> s): Node(Lav_OBJTYPE_RINGMOD_NODE, s, 2, 1) {
	appendInputConnection(0, 1);
	appendInputConnection(1, 1);
	appendOutputConnection(0, 1);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createRingmodNode(std::shared_ptr<Server> server) {
	return standardNodeCreation<RingmodNode>(server);
}

void RingmodNode::process() {
	multiplicationKernel(block_size, input_buffers[0], input_buffers[1], output_buffers[0]);
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createRingmodNode(LavHandle serverHandle, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	*destination = outgoingObject<Node>(createRingmodNode(server));
	PUB_END
}

}