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
}

void LavSimulation::doMaintenance() {
	decltype(nodes) to_remove;
	for(auto &n: nodes) {
		if(n.lock() == nullptr) to_remove.insert(n);
	}
	for(auto &n: to_remove) {
		nodes.erase(n);
	}
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
	return Lav_ERROR_NONE;
}

std::shared_ptr<LavNode> LavSimulation::getOutputNode() {
	return output_node.lock();
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
