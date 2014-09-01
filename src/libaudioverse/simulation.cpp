/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_audio_devices.hpp>
#include <stdlib.h>
#include <functional>
#include <algorithm>
#include <iterator>
#include <thread>

LavSimulation::LavSimulation(unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixahead) {
	this->sr = (float)sr;
	this->channels = channels;
	this->block_size = blockSize;
	this->mixahead = mixahead;
	//fire up the background thread.
	backgroundTaskThread = std::thread([this]() {backgroundTaskThreadFunction();});
	start();
}

LavSimulation::~LavSimulation() {
	//enqueue a task which will stop the background thread.
	enqueueTask([]() {throw LavThreadTerminationException();});
	backgroundTaskThread.join();
}

LavError LavSimulation::getBlock(float* out) {
	//if paused, memset 0s.
	if(is_started == 0) {
		memset(out, 0, sizeof(float)*channels*block_size);
		return Lav_ERROR_NONE;
	}
	//if no output object, memset 0 and bail out again.
	if(output_object == nullptr) {
		memset(out, 0, sizeof(float)*channels*block_size);
		return Lav_ERROR_NONE;
	}
	//okay, we're not paused.  Visit all objects in the order that they would be processed, and record it.
	process_order.clear();
	visitAllObjectsInProcessOrder([this] (std::shared_ptr<LavObject> o) {
		process_order.push_back(o);
	});
	//visit all objects in reverse order.
	//an object to the right in the vector depends on some subset of objects to its left, but never anything to its right.
	for(auto i = process_order.rbegin(); i != process_order.rend(); i++) {
		(*i)->willProcessParents();
	}
	//visit all objects in order, and do the processing.
	for(auto obj: process_order) {
		obj->willProcess();
		obj->process();
		obj->didProcess();
	}
	if(output_object->getState() == Lav_OBJECT_STATE_PAUSED) { //fast path, just zero.
		memset(out, 0, sizeof(float)*block_size*channels);
		return Lav_ERROR_NONE;
	}
	float** outputs = new float*[output_object->getOutputCount()];
	output_object->getOutputPointers(outputs);
	//interweave them.
	for(unsigned int i = 0; i < block_size*channels; i++) {
		out[i] = i%channels < output_object->getOutputCount() ? outputs[i%channels][i/channels] : 0.0f; //i%channels is the channel this sample belongs to; i/channels is the position in the i%channelsth output.
	}
	delete[] outputs;
	return Lav_ERROR_NONE;
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
	//visit the rest: lav_OBJECT_STATE_ALWAYS_PLAYING.
	for(auto& i: objects) {
		std::shared_ptr<LavObject> j = i.lock();
		if(j->getState() == Lav_OBJECT_STATE_ALWAYS_PLAYING) {
			visitForProcessing(j, visitorWrapped);
		}
	}
}

void LavSimulation::visitForProcessing(std::shared_ptr<LavObject> obj, std::function<void(std::shared_ptr<LavObject>)> visitor) {
	//if obj is null, bail out.  This is the base case.
	if(obj == nullptr) return;
	//if the object is paused, we also bail out: this object and its parents are not needed.
	if(obj->getState() == Lav_OBJECT_STATE_PAUSED) return;
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

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlock(LavSimulation* simulation, float* destination) {
	PUB_BEGIN
	LOCK(*simulation);
	simulation->getBlock(destination);
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

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetChannels(LavSimulation* simulation, int* destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = simulation->getChannels();
	PUB_END
}
