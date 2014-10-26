/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_kernels.hpp>
#include <libaudioverse/private_audio_devices.hpp>
#include <libaudioverse/private_data.hpp>
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

void LavSimulation::getBlock(float* out, unsigned int channels, bool mayApplyMixingMatrix) {
	if(channels == 0) return;
	//if paused, memset 0s.
	if(is_started == 0) {
		memset(out, 0, sizeof(float)*channels*block_size);
		return;
	}
	//replan, if needed.
	if(planInvalidated) {
		replan();
		planInvalidated = false;
	}
	//visit all objects in reverse order.
	//an object to the right in the vector depends on some subset of objects to its left, but never anything to its right.
	for(auto i = plan.rbegin(); i != plan.rend(); i++) {
		(*i)->willProcessParents();
	}
	//visit all objects in order, and do the processing.
	for(auto obj: plan) {
		obj->willProcess();
		obj->process();
		obj->didProcess();
	}
	if(output_object == nullptr || output_object->getState() == Lav_OBJSTATE_PAUSED) { //fast path, just zero.
		memset(out, 0, sizeof(float)*block_size*channels);
		return;
	}

	float *mixingMatrix = getMixingMatrix(output_object->getOutputCount(), channels);
	if(mixingMatrix && mayApplyMixingMatrix) {
		for(unsigned int i = 0; i < output_object->getOutputCount(); i++) {
			std::copy(output_object->getOutputPointer(i), output_object->getOutputPointer(i)+block_size, mixing_matrix_workspace+i*block_size);
		}
		interleaveSamples(output_object->getOutputCount(), block_size, mixing_matrix_workspace);
		applyMixingMatrix(output_object->getOutputCount()*block_size, mixing_matrix_workspace, out, output_object->getOutputCount(), channels, mixingMatrix);
	}
	else {
		for(unsigned int i = 0; i < channels; i++) {
			if(i < output_object->getOutputCount()) memcpy(out+i*getBlockSize(), output_object->getOutputPointer(i), sizeof(float)*getBlockSize());
			else memset(out, 0, sizeof(float)*getBlockSize()); //we're beyond the number of inputs.
		}
		interleaveSamples(channels, getBlockSize(), out);
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

LavError LavSimulation::associateObject(std::shared_ptr<LavObject> obj) {
	objects.insert(std::weak_ptr<LavObject>(obj));
	return Lav_ERROR_NONE;
}

LavError LavSimulation::setOutputObject(std::shared_ptr<LavObject> obj) {
	output_object = obj;
	invalidatePlan();
	return Lav_ERROR_NONE;
}

std::shared_ptr<LavObject> LavSimulation::getOutputObject() {
	return output_object;
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
	visitAllObjectsInProcessOrder([this] (std::shared_ptr<LavObject> o) {
		plan.push_back(o);
	});
}

void LavSimulation::visitAllObjectsInProcessOrder(std::function<void(std::shared_ptr<LavObject>)> visitor) {
	std::set<std::shared_ptr<LavObject>> seen;
	auto visitorWrapped = [&](std::shared_ptr<LavObject> o) {
		if(seen.count(o)) return;
		visitor(o);
		seen.insert(o);
	};
	if(output_object) {
		visitForProcessing(output_object, visitorWrapped);
	}
	//visit the rest: Lav_OBJSTATE_ALWAYS_PLAYING.
	for(auto& i: objects) {
		std::shared_ptr<LavObject> j = i.lock();
		if(j->getState() == Lav_OBJSTATE_ALWAYS_PLAYING) {
			visitForProcessing(j, visitorWrapped);
		}
	}
}

void LavSimulation::visitForProcessing(std::shared_ptr<LavObject> obj, std::function<void(std::shared_ptr<LavObject>)> visitor) {
	//if obj is null, bail out.  This is the base case.
	if(obj == nullptr) return;
	//if the object is paused, we also bail out: this object and its parents are not needed.
	if(obj->getState() == Lav_OBJSTATE_PAUSED) return;
	//we call ourselves on all parents of obj, and then pass obj to visitor.  This is essentially depth-first search.
	for(unsigned int i = 0; i < obj->getParentCount(); i++) {
		visitForProcessing(obj->getParentObject(i), visitor);
	}
	visitor(obj);
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

Lav_PUBLIC_FUNCTION LavError Lav_simulationSetOutputObject(LavSimulation* simulation, LavObject* object) {
	PUB_BEGIN
	LOCK(*simulation);
	simulation->setOutputObject(incomingPointer<LavObject>(object));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetOutputObject(LavSimulation* simulation, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = outgoingPointer<LavObject>(simulation->getOutputObject());
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
