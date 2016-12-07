/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/hard_limiter.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <memory>

namespace libaudioverse_implementation {

HardLimiterNode::HardLimiterNode(std::shared_ptr<Server> server, int channels): Node(Lav_OBJTYPE_HARD_LIMITER_NODE, server, channels, channels) {
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node>createHardLimiterNode(std::shared_ptr<Server> server, int channels) {
	return standardNodeCreation<HardLimiterNode>(server, channels);
}

void HardLimiterNode::process() {
	for(unsigned int i = 0; i < block_size; i++) {
		for(unsigned int o = 0; o < num_output_buffers; o++) {
			if(input_buffers[o][i] > 1.0f) {
				output_buffers[o][i] = 1.0f;
				continue;
			}
			else if(input_buffers[o][i] < -1.0f) {
				output_buffers[o][i] = -1.0f;
				continue;
			}
			output_buffers[o][i] = input_buffers[o][i];
		}
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createHardLimiterNode(LavHandle serverHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server =incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createHardLimiterNode(server, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}