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
#include <libaudioverse/nodes/convolver.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/implementations/convolvers.hpp>
#include <limits>

namespace libaudioverse_implementation {

ConvolverNode::ConvolverNode(std::shared_ptr<Server> server, int channels): Node(Lav_OBJTYPE_CONVOLVER_NODE, server, channels, channels) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	appendInputConnection(0, channels);
	this->channels=channels;
	appendOutputConnection(0, channels);
	convolvers=new BlockConvolver*[channels]();
	for(int i= 0; i < channels; i++) convolvers[i] = new BlockConvolver(server->getBlockSize());
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createConvolverNode(std::shared_ptr<Server> server, int channels) {
	return standardNodeCreation<ConvolverNode>(server, channels);
}

ConvolverNode::~ConvolverNode() {
	for(int i = 0; i < channels; i++) delete convolvers[i];
	delete[] convolvers;
}

void ConvolverNode::process() {
	if(werePropertiesModified(this, Lav_CONVOLVER_IMPULSE_RESPONSE)) setImpulseResponse();
	for(int i= 0; i < channels; i++) convolvers[i]->convolve(input_buffers[i], output_buffers[i]);
}

void ConvolverNode::setImpulseResponse() {
	auto ir=getProperty(Lav_CONVOLVER_IMPULSE_RESPONSE).getFloatArrayPtr();
	int len =getProperty(Lav_CONVOLVER_IMPULSE_RESPONSE).getFloatArrayLength();
	for(int i = 0; i < channels; i++) convolvers[i]->setResponse(len, ir);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createConvolverNode(LavHandle serverHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createConvolverNode(server, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}