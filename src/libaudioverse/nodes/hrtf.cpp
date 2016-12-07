/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/hrtf.hpp>
#include <libaudioverse/implementations/hrtf_panner.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/constants.hpp>
#include <memory>

namespace libaudioverse_implementation {

HrtfNode::HrtfNode(std::shared_ptr<Server> server, std::shared_ptr<HrtfData> hrtf): Node(Lav_OBJTYPE_HRTF_NODE, server, 1, 2),
panner(server->getBlockSize(), server->getSr(), hrtf) {
	appendInputConnection(0, 1);
	appendOutputConnection(0, 2);
}

std::shared_ptr<Node>createHrtfNode(std::shared_ptr<Server>server, std::shared_ptr<HrtfData> hrtf) {
	return standardNodeCreation<HrtfNode>(server, hrtf);
}

void HrtfNode::process() {
	panner.setAzimuth(getProperty(Lav_PANNER_AZIMUTH).getFloatValue());
	panner.setElevation(getProperty(Lav_PANNER_ELEVATION).getFloatValue());
	panner.setShouldCrossfade(getProperty(Lav_PANNER_SHOULD_CROSSFADE).getIntValue() == 1);
	panner.pan(input_buffers[0], output_buffers[0], output_buffers[1]);
}

void HrtfNode::reset() {
	panner.reset();
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfNode(LavHandle serverHandle, const char* hrtfPath, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto hrtf = createHrtfFromString(hrtfPath, server->getSr());
	auto retval = createHrtfNode(server, hrtf);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}