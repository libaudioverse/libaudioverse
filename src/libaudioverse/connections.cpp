/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Handles functionality common to all objects: linking, allocating, and freeing, as well as parent-child relationships.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/connections.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/helper_templates.hpp>
#include <audio_io/audio_io.hpp>
#include <algorithm>
#include <memory>
#include <vector>


namespace libaudioverse_implementation {

OutputConnection::OutputConnection(std::shared_ptr<Simulation> simulation, Node* node, int start, int count) {
	this->simulation = simulation;
	this->node= node;
	this->start =start;
	this->count = count;
	this->block_size = simulation->getBlockSize();
}

void OutputConnection::add(int inputBufferCount, float** inputBuffers, bool shouldApplyMixingMatrix) {
	//Ticking is now handled by the planner, see planner.cpp.
	//If the node is paused, we are going to output zeros, so skip.
	if(node->getState() == Lav_NODESTATE_PAUSED) return;
	//get the array of outputs from our node.
	float** outputArray=node->getOutputBufferArray();
	//it is the responsibility of our node to keep us configured, so we assume what info we have is accurate. If it is not, that is the fault of our node.
	///if they're the same, we fall through because we can do better with our sse kernels than audio_io.
	if(shouldApplyMixingMatrix && inputBufferCount != count ) {
		//Remix, but don't zero first.
		audio_io::remixAudioUninterleaved(block_size, count, outputArray+start, inputBufferCount, inputBuffers, false);
	}
	else { //copy and drop.
		int channelsToAdd =std::min(count, inputBufferCount);
		for(int i=0; i < channelsToAdd; i++) additionKernel(block_size, outputArray[i+start], inputBuffers[i], inputBuffers[i]);
	}
}

void OutputConnection::reconfigure(int newStart, int newCount) {
	start=newStart;
	count= newCount;
}

void OutputConnection::clear() {
	for(auto &c: connected_to) {
		auto c_s= c.lock();
		if(c_s) c_s->forgetConnection(this);
	}
	//and then kill the whole set.
	connected_to.clear();
}

void OutputConnection::connectHalf(std::shared_ptr<InputConnection> inputConnection) {
	connected_to.insert(inputConnection);
}

void OutputConnection::disconnectHalf(std::shared_ptr<InputConnection> connection) {
	if(connected_to.count(connection)) {
		connected_to.erase(connection);
	}
}

Node* OutputConnection::getNode() {
	return node;
}

std::vector<Node*> OutputConnection::getConnectedNodes() {
	std::vector<Node*> retval;
	filterWeakPointers(connected_to, [&](std::shared_ptr<InputConnection> &conn) {
		auto n = conn->getNode();
		//The simulation uses an input connection without a node, so we need to protect against this case.
		if(n) retval.push_back(n);
	});
	return retval;
}

InputConnection::InputConnection(std::shared_ptr<Simulation> simulation, Node* node, int start, int count) {
	this->simulation = simulation;
	this->node= node;
	this->start=start;
	this->count = count;
	this->block_size = simulation->getBlockSize();
}

void InputConnection::add(bool shouldApplyMixingMatrix) {
	addNodeless(node->getInputBufferArray(), shouldApplyMixingMatrix);
}

void InputConnection::addNodeless(float** inputs, bool shouldApplyMixingMatrix) {
	for(auto &i: connected_to) {
		i.first->add(count, inputs+start, shouldApplyMixingMatrix);
	}
}

void InputConnection::reconfigure(int newStart, int newCount) {
	start= newStart;
	count= newCount;
}

void InputConnection::connectHalf(std::shared_ptr<OutputConnection> outputConnection) {
	auto n = std::static_pointer_cast<Node>(outputConnection->getNode()->shared_from_this());
	connected_to[outputConnection] = n;
}

void InputConnection::disconnectHalf(std::shared_ptr<OutputConnection> connection) {
	if(connected_to.count(connection)) {
		connected_to.erase(connection);
	}
}

void InputConnection::forgetConnection(OutputConnection* which) {
	filter(connected_to, [](decltype(connected_to)::value_type &k, OutputConnection* t)->bool {
		return k.first.get() != t;
	}, which);
}

Node* InputConnection::getNode() {
	return node;
}

std::vector<Node*> InputConnection::getConnectedNodes() {
	std::vector<Node*> retval;
	for(auto &i: connected_to) {
		auto n= i.first->getNode();
		retval.push_back(n);
	}
	return retval;
}

int InputConnection::getConnectedNodeCount() {
	return (int)getConnectedNodes().size();
}

void makeConnection(std::shared_ptr<OutputConnection> output, std::shared_ptr<InputConnection> input) {
	output->connectHalf(input);
	input->connectHalf(output);
}

void breakConnection(std::shared_ptr<OutputConnection> output, std::shared_ptr<InputConnection> input) {
	output->disconnectHalf(input);
	input->disconnectHalf(output);
}

}