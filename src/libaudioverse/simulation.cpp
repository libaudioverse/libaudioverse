/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/data.hpp>
#include <stdlib.h>
#include <functional>
#include <algorithm>
#include <iterator>
#include <thread>
#include <tuple>
#include <map>

LavSimulation::LavSimulation(unsigned int sr, unsigned int blockSize, unsigned int mixahead) {
	this->sr = (float)sr;
	this->block_size = blockSize;
	this->mixahead = mixahead;
	registerDefaultMixingMatrices();
	//fire up the background thread.
	backgroundTaskThread = std::thread([this]() {backgroundTaskThreadFunction();});
	start();
}

LavSimulation::~LavSimulation() {
	//enqueue a task which will stop the background thread.
	enqueueTask([]() {throw LavThreadTerminationException();});
	backgroundTaskThread.join();
}

//Yes, this uses goto. Yes, goto is evil. We need a single point of exit.
void LavSimulation::getBlock(float* out, unsigned int channels, bool mayApplyMixingMatrix) {
	if(channels == 0) return;
	//if paused, memset 0s.
	if(is_started == 0) {
		memset(out, 0, sizeof(float)*channels*block_size);
		goto end;
	}
	//try to restore the plan, at least unless it is invalidated.
	if(planInvalidated == false) {
		for(auto i: weak_plan) {
			auto j= i.lock();
	if(j== nullptr) {
				planInvalidated = true;
				break;	
			}
			plan.push_back(j);
		}
	}
	//this is here because the above block can also invalidate the plan.
	//replan, if needed.
	if(planInvalidated) {
		replan();
		planInvalidated = false;
		weak_plan.clear();
		std::copy(plan.begin(), plan.end(), std::inserter(weak_plan, weak_plan.end()));
	}
	//visit all nodesin reverse order.
	//a nodeto the right in the vector depends on some subset of objects to its left, but never anything to its right.
	for(auto i = plan.rbegin(); i != plan.rend(); i++) {
		(*i)->willProcessParents();
	}
	//visit all nodes in order, and do the processing.
	for(auto obj: plan) {
		obj->doProcessProtocol();
	}
	//we're done with the strong plan, so kill it.
	//this lets objects delete.
	plan.clear();
	if(output_node== nullptr || output_node->getState() == Lav_NODESTATE_PAUSED) { //fast path, just zero.
		memset(out, 0, sizeof(float)*block_size*channels);
		goto end;
	}

	float *mixingMatrix = getMixingMatrix(output_node->getOutputCount(), channels);
	float** outputPointers = new float*[output_node->getOutputCount()];
	output_node->getOutputPointers(outputPointers);
	if(mixingMatrix && mayApplyMixingMatrix) {
		interleaveSamples(channels, getBlockSize(), output_node->getOutputCount(), outputPointers, mixing_matrix_workspace);
		applyMixingMatrix(output_node->getOutputCount()*block_size, mixing_matrix_workspace, out, output_node->getOutputCount(), channels, mixingMatrix);
	}
	else {
		interleaveSamples(channels, getBlockSize(), output_node->getOutputCount(), outputPointers, out);
	}
	delete[] outputPointers;
	end:
	tick_count++;
}

LavError LavSimulation::start() {
	is_started = 1;
	return Lav_ERROR_NONE;
}

LavError LavSimulation::stop() {
	is_started = 0;
	return Lav_ERROR_NONE;
}

LavError LavSimulation::associateNode(std::shared_ptr<LavNode> node) {
	nodes.insert(std::weak_ptr<LavNode>(node));
	return Lav_ERROR_NONE;
}

LavError LavSimulation::setOutputNode(std::shared_ptr<LavNode> node) {
	output_node= node;
	invalidatePlan();
	return Lav_ERROR_NONE;
}

std::shared_ptr<LavNode> LavSimulation::getOutputNode() {
	return output_node;
}

void LavSimulation::enqueueTask(std::function<void(void)> cb) {
	tasks.enqueue(cb);
}

void LavSimulation::associateDevice(std::shared_ptr<LavDevice> what) {
	device = what;
}

void LavSimulation::registerMixingMatrix(unsigned int inChannels, unsigned int outChannels, float* matrix) {
	mixing_matrices[std::tuple<unsigned int, unsigned int>(inChannels, outChannels)] = matrix;
	if(inChannels > largest_seen_mixing_matrix_input) {
		if(mixing_matrix_workspace) delete[] mixing_matrix_workspace;
		mixing_matrix_workspace = new float[block_size*inChannels]();
		largest_seen_mixing_matrix_input = inChannels;
	}
}

void LavSimulation::resetMixingMatrix(unsigned int  inChannels, unsigned int outChannels) {
	for(LavMixingMatrixInfo* i = mixing_matrix_list; i->pointer; i++) {
		if(i->in_channels == inChannels && i->out_channels == outChannels) {
			mixing_matrices[std::tuple<unsigned int, unsigned int>(inChannels, outChannels)] = i->pointer;
			return;
		}
	}
	if(mixing_matrices.count(std::tuple<unsigned int, unsigned int>(inChannels, outChannels)) != 0) mixing_matrices.erase(std::tuple<unsigned int, unsigned int>(inChannels, outChannels));
}

void LavSimulation::registerDefaultMixingMatrices() {
	for(LavMixingMatrixInfo* i = mixing_matrix_list; i->pointer; i++) registerMixingMatrix(i->in_channels, i->out_channels, i->pointer);
}

float* LavSimulation::getMixingMatrix(unsigned int inChannels, unsigned int outChannels) {
	std::tuple<unsigned int, unsigned int> key(inChannels, outChannels);
	if(mixing_matrices.count(key) != 0) return mixing_matrices[key];
	else return nullptr;
}

void LavSimulation::invalidatePlan() {
	planInvalidated = true;
}

void LavSimulation::replan() {
	plan.clear();
	visitAllNodesInProcessOrder([this] (std::shared_ptr<LavNode> n) {
		plan.push_back(n);
	});
}

void LavSimulation::visitAllNodesInProcessOrder(std::function<void(std::shared_ptr<LavNode>)> visitor) {
	std::set<std::shared_ptr<LavNode>> seen;
	auto visitorWrapped = [&](std::shared_ptr<LavNode> n) {
		if(seen.count(n)) return;
		visitor(n);
		seen.insert(n);
	};
	if(output_node) {
		visitForProcessing(output_node, visitorWrapped);
	}
	//visit the rest: Lav_NODESTATE_ALWAYS_PLAYING.
	for(auto& i: nodes) {
		std::shared_ptr<LavNode> j = i.lock();
		if(j == nullptr) return; //this is dead.
		if(j->getState() == Lav_NODESTATE_ALWAYS_PLAYING) {
			visitForProcessing(j, visitorWrapped);
		}
	}
}

void LavSimulation::visitForProcessing(std::shared_ptr<LavNode> node, std::function<void(std::shared_ptr<LavNode>)> visitor) {
	//if node is null, bail out.  This is the base case.
	if(node == nullptr) return;
	//if the node is paused, we also bail out: this object and its parents are not needed.
	if(node->getState() == Lav_NODESTATE_PAUSED) return;
	//we call ourselves on all parents of node, and then pass node to visitor.  This is essentially depth-first search.
	for(unsigned int i = 0; i < node->getParentCount(); i++) {
		visitForProcessing(node->getParentNode(i), visitor);
	}
	visitor(node);
}

//Default callback implementation.
void LavSimulation::backgroundTaskThreadFunction() {
	try {
		for(;;) {
			auto task = tasks.dequeue();
			task();
		}
	}
	catch(LavThreadTerminationException) {
		return;
	}
}

//begin public API

Lav_PUBLIC_FUNCTION LavError Lav_simulationSetOutputNode(LavSimulation* simulation, LavNode* node) {
	PUB_BEGIN
	LOCK(*simulation);
	simulation->setOutputNode(incomingPointer<LavNode>(node));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetOutputNode(LavSimulation* simulation, LavNode** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = outgoingPointer<LavNode>(simulation->getOutputNode());
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlock(LavSimulation* simulation, unsigned int channels, int mayApplyMixingMatrix, float* destination) {
	PUB_BEGIN
	LOCK(*simulation);
	simulation->getBlock(destination, channels, mayApplyMixingMatrix != 0);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlockSize(LavSimulation* simulation, int* destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = simulation->getBlockSize();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetSr(LavSimulation* simulation, int* destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = (int)simulation->getSr();
	PUB_END
}

//atomic blocks

Lav_PUBLIC_FUNCTION LavError Lav_simulationBeginAtomicBlock(LavSimulation *simulation) {
	PUB_BEGIN
	simulation->lock();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationEndAtomicBlock(LavSimulation* simulation) {
	PUB_BEGIN
	simulation->unlock();
	PUB_END
}
