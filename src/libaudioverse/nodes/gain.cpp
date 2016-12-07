/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/gain.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

GainNode::GainNode(std::shared_ptr<Server> s): Node(Lav_OBJTYPE_GAIN_NODE, s, 0, 0) {
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createGainNode(std::shared_ptr<Server> server) {
	return standardNodeCreation<GainNode>(server);
}

void GainNode::process() {
	for(unsigned int i = 0; i < input_buffers.size(); i++) {
		std::copy(input_buffers[i], input_buffers[i]+block_size, output_buffers[i]);
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createGainNode(LavHandle serverHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval= createGainNode(server);
	retval->resize(channels, channels);
	retval->appendInputConnection(0, channels);
	retval->appendOutputConnection(0, channels);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

}