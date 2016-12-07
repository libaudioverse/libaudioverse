/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/iir.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/iir.hpp>

namespace libaudioverse_implementation {

IirNode::IirNode(std::shared_ptr<Server> server, int channels): Node(Lav_OBJTYPE_IIR_NODE, server, channels, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	this->channels=channels;
	filters=new IIRFilter*[channels]();
	for(int i= 0; i < channels; i++) filters[i] = new IIRFilter(server->getSr());
	double defaultNumerator[] = {1.0};
	double defaultDenominator[] = {1.0, 0.0}; //identity filter.
	setCoefficients(1, defaultNumerator, 2, defaultDenominator, 1);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createIirNode(std::shared_ptr<Server> server, int channels) {
	return standardNodeCreation<IirNode>(server, channels);
}

IirNode::~IirNode() {
	for(int i = 0; i < channels; i++) delete filters[i];
	delete[] filters;
}

void IirNode::process() {
	for(unsigned int i = 0; i < channels; i++) {
		auto &f = *filters[i];
		for(int j = 0; j < block_size; j++) output_buffers[i][j] = f.tick(input_buffers[i][j]);
	}
}

void IirNode::setCoefficients(int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory) {
	for(int i =0; i < channels; i++) {
		filters[i]->configure(numeratorLength, numerator, denominatorLength, denominator);
		if(shouldClearHistory !=0) filters[i]->clearHistories();
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createIirNode(LavHandle serverHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto server = incomingObject<Server>(serverHandle);
	LOCK(*server);
	auto retval = createIirNode(server, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_iirNodeSetCoefficients(LavHandle nodeHandle, int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_IIR_NODE) ERROR(Lav_ERROR_TYPE_MISMATCH, "Expected an IIR node.");
	std::static_pointer_cast<IirNode>(node)->setCoefficients(numeratorLength, numerator, denominatorLength, denominator, shouldClearHistory);
	PUB_END
}

}