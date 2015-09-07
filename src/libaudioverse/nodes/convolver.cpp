/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
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

class ConvolverNode: public Node {
	public:
	ConvolverNode(std::shared_ptr<Simulation> simulation, int channels);
	~ConvolverNode();
	virtual void process();
	void setImpulseResponse();
	int channels;
	BlockConvolver **convolvers;
};

ConvolverNode::ConvolverNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_CONVOLVER_NODE, simulation, channels, channels) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	appendInputConnection(0, channels);
	this->channels=channels;
	appendOutputConnection(0, channels);
	convolvers=new BlockConvolver*[channels]();
	for(int i= 0; i < channels; i++) convolvers[i] = new BlockConvolver(simulation->getBlockSize());
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