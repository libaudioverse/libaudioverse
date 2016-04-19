/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/iir.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/iir.hpp>

namespace libaudioverse_implementation {

IirNode::IirNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_IIR_NODE, simulation, channels, channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	this->channels=channels;
	filters=new IIRFilter*[channels]();
	for(int i= 0; i < channels; i++) filters[i] = new IIRFilter(simulation->getSr());
	double defaultNumerator[] = {1.0};
	double defaultDenominator[] = {1.0, 0.0}; //identity filter.
	setCoefficients(1, defaultNumerator, 2, defaultDenominator, 1);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	setShouldZeroOutputBuffers(false);
}

std::shared_ptr<Node> createIirNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<IirNode>(simulation, channels);
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

Lav_PUBLIC_FUNCTION LavError Lav_createIirNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createIirNode(simulation, channels);
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