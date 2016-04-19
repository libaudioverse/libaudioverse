/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/convolver.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/implementations/convolvers.hpp>
#include <limits>

namespace libaudioverse_implementation {

ConvolverNode::ConvolverNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_CONVOLVER_NODE, simulation, channels, channels) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	appendInputConnection(0, channels);
	this->channels=channels;
	appendOutputConnection(0, channels);
	convolvers=new BlockConvolver*[channels]();
	for(int i= 0; i < channels; i++) convolvers[i] = new BlockConvolver(simulation->getBlockSize());
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createConvolverNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<ConvolverNode>(simulation, channels);
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

Lav_PUBLIC_FUNCTION LavError Lav_createConvolverNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createConvolverNode(simulation, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}