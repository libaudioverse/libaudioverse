/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/multichannel_filter_bank.hpp>
#include <libaudioverse/implementations/nested_allpass_network.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

class NestedAllpassNetworkNode: public Node {
	public:
	NestedAllpassNetworkNode(std::shared_ptr<Simulation> sim, int channels);
	void process() override;
	void reset() override;
	void beginNesting(int delay, float coefficient);
	void endNesting();
	void appendAllpass(int delay, float coefficient);
	void appendOnePole(float frequency, bool isHighpass = false);
	void appendBiquad(int type, double frequency, double dbGain, double q);
	void appendReader(float mul);
	void compile();
	MultichannelFilterBank<NestedAllpassNetwork> bank;
};

NestedAllpassNetworkNode::NestedAllpassNetworkNode(std::shared_ptr<Simulation> sim, int channels): Node(Lav_OBJTYPE_NESTED_ALLPASS_NETWORK_NODE, sim, channels, channels),
bank(simulation->getSr()) {
	if(channels < 1) ERROR(Lav_ERROR_RANGE, "Cannot filter 0 or fewer channels.");
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	bank.setChannelCount(channels);
}

std::shared_ptr<Node> createNestedAllpassNetworkNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<NestedAllpassNetworkNode>(simulation, channels);
}

void NestedAllpassNetworkNode::process() {
	bank.process(block_size, &input_buffers[0], &output_buffers[0]);
}

void NestedAllpassNetworkNode::reset() {
	bank.reset();
}

void NestedAllpassNetworkNode::beginNesting(int delay, float coefficient) {
	bank->beginNesting(delay, coefficient);
}

void NestedAllpassNetworkNode::endNesting() {
	bank->endNesting();
}

void NestedAllpassNetworkNode::appendAllpass(int delay, float coefficient) {
	bank->appendAllpass(delay, coefficient);
}

void NestedAllpassNetworkNode::appendOnePole(float frequency, bool isHighpass) {
	bank->appendOnePole(frequency, isHighpass);
}

void NestedAllpassNetworkNode::appendBiquad(int type, double frequency, double dbGain, double q) {
	bank->appendBiquad(type, frequency, dbGain, q);
}

void NestedAllpassNetworkNode::appendReader(float mul) {
	bank->appendReader(mul);
}

void NestedAllpassNetworkNode::compile() {
	bank->compile();
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createNestedAllpassNetworkNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval= createNestedAllpassNetworkNode(simulation, channels);
	*destination =outgoingObject<Node>(retval);
	PUB_END
}

//Note the space after PUB_BEGIN, it's necessary.
#define PRE \
PUB_BEGIN \
auto n = incomingObject<NestedAllpassNetworkNode>(nodeHandle);\
LOCK(*n);

Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeBeginNesting(LavHandle nodeHandle, int delay, float coefficient) {
	PRE
	n->beginNesting(delay, coefficient);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeEndNesting(LavHandle nodeHandle) {
	PRE
	n->endNesting();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeAppendAllpass(LavHandle nodeHandle, int delay, float coefficient) {
	PRE
	n->appendAllpass(delay, coefficient);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeAppendOnePole(LavHandle nodeHandle, float frequency, int isHighpass) {
	PRE
	n->appendOnePole(frequency, isHighpass == 1);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeAppendBiquad(LavHandle nodeHandle, int type, double frequency, double dbGain, double q) {
	PRE
	n->appendBiquad(type, frequency, dbGain, q);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeAppendReader(LavHandle nodeHandle, float mul) {
	PRE
	n->appendReader(mul);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_nestedAllpassNetworkNodeCompile(LavHandle nodeHandle) {
	PRE
	n->compile();
	PUB_END
}

}