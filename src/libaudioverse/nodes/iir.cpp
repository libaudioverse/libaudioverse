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
#include <libaudioverse/private/functiontables.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <limits>
#include <libaudioverse/private/iir.hpp>

class LavIirNode: public LavNode {
	public:
	LavIirNode(std::shared_ptr<LavSimulation> simulation, int channels);
	virtual void process();
	void setCoefficients(int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory);
	std::vector<LavIIRFilter> filters;
};

LavIirNode::LavIirNode(std::shared_ptr<LavSimulation> simulation, int channels): LavNode(Lav_NODETYPE_IIR, simulation, channels, channels) {
	if(channels <= 0) throw LavErrorException(Lav_ERROR_RANGE);
	filters.resize(channels);
	double defaultNumerator[] = {1.0};
	double defaultDenominator[] = {1.0, 0.0}; //identity filter.
	setCoefficients(1, defaultNumerator, 2, defaultDenominator, 1);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<LavNode> createIirNode(std::shared_ptr<LavSimulation> simulation, int channels) {
	std::shared_ptr<LavIirNode> retval = std::shared_ptr<LavIirNode>(new LavIirNode(simulation, channels), LavObjectDeleter);
	simulation->associateNode(retval);
	return retval;
}

void LavIirNode::process() {
	for(unsigned int i = 0; i < filters.size(); i++) {
		auto &f =filters[i];
		for(int j = 0; j < block_size; j++) output_buffers[i][j] = f.tick(input_buffers[i][j]);
	}
}

void LavIirNode::setCoefficients(int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory) {
	for(auto &i: filters) {
		i.configure(numeratorLength, numerator, denominatorLength, denominator);
		if(shouldClearHistory !=0) i.clearHistories();
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createIirNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createIirNode(simulation, channels);
	*destination = outgoingObject<LavNode>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_iirNodeSetCoefficients(LavHandle nodeHandle, int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory) {
	PUB_BEGIN
	auto node = incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_NODETYPE_IIR) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	std::static_pointer_cast<LavIirNode>(node)->setCoefficients(numeratorLength, numerator, denominatorLength, denominator, shouldClearHistory);
	PUB_END
}