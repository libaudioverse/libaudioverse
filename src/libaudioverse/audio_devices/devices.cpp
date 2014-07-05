/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <stdlib.h>
#include <functional>
#include <algorithm>
#include <iterator>

LavDevice::LavDevice(unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixahead) {
	this->sr = (float)sr;
	this->channels = channels;
	this->block_size = blockSize;
	this->mixahead = mixahead;
	start();
}

LavError LavDevice::getBlock(float* out) {
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
	float** outputs = new float*[output_object->getOutputCount()];
	output_object->getOutputPointers(outputs);
	//interweave them.
	for(unsigned int i = 0; i < block_size*channels; i++) {
		out[i] = i%channels < output_object->getOutputCount() ? outputs[i%channels][i/channels] : 0.0f; //i%channels is the channel this sample belongs to; i/channels is the position in the i%channelsth output.
	}
	delete[] outputs;
	return Lav_ERROR_NONE;
}

LavError LavDevice::start() {
	is_started = 1;
	return Lav_ERROR_NONE;
}

LavError LavDevice::stop() {
	is_started = 0;
	return Lav_ERROR_NONE;
}

LavError LavDevice::associateObject(std::shared_ptr<LavObject> obj) {
	objects.insert(std::weak_ptr<LavObject>(obj));
	return Lav_ERROR_NONE;
}

LavError LavDevice::setOutputObject(std::shared_ptr<LavObject> obj) {
	output_object = obj;
	return Lav_ERROR_NONE;
}

std::shared_ptr<LavObject> LavDevice::getOutputObject() {
	return output_object;
}

void LavDevice::visitAllObjectsInProcessOrder(std::function<void(std::shared_ptr<LavObject>)> visitor) {
	std::set<std::shared_ptr<LavObject>> seen;
	if(output_object) {
		visitForProcessing(output_object, [&](std::shared_ptr<LavObject> o) {
			if(seen.count(o)) return;
			visitor(o);
			seen.insert(o);
		});
	}
	std::set<std::shared_ptr<LavObject>> still_needed, always_process_shared;
	for(auto i: always_process) {
		auto tmp = i.lock();
		if(tmp == nullptr) continue;
		always_process_shared.insert(tmp);
	}
	do {
		still_needed.clear();
		std::set_difference(always_process_shared.begin(), always_process_shared.end(), seen.begin(), seen.end(),
			std::inserter(still_needed, still_needed.end()));
		for(auto i = still_needed.begin(); i != still_needed.end(); i++) {
			visitForProcessing(*i, [&] (std::shared_ptr<LavObject> o) {
				if(seen.count(o) == 0) {
					visitor(o);
					seen.insert(o);
				}
			});
		}
	} while(still_needed.size());
	always_process.clear();
	//this keeps us from building up control blocks, etc.
	//may be premature optimization, but doesn't hurt.
	for(auto i: always_process_shared) {
		always_process.insert(i);
	}
}

void LavDevice::visitForProcessing(std::shared_ptr<LavObject> obj, std::function<void(std::shared_ptr<LavObject>)> visitor) {
	//if obj is null, bail out.  This is the base case.
	if(obj == nullptr) return;
	//if the object is suspended, we also bail out: this object and its parents are not needed.
	if(obj->isSuspended()) return;
	//we call ourselves on all parents of obj, and then pass obj to visitor.  This is essentially depth-first search.
	for(unsigned int i = 0; i < obj->getInputCount(); i++) {
		visitForProcessing(obj->getParentObject(i), visitor);
	}
	visitor(obj);
}

//begin public API

Lav_PUBLIC_FUNCTION LavError Lav_deviceSetOutputObject(LavDevice* device, LavObject* object) {
	LOCK(*device);
	device->setOutputObject(incomingPointer<LavObject>(object));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetOutputObject(LavDevice* device, LavObject** destination) {
	LOCK(*device);
	*destination = outgoingPointer<LavObject>(device->getOutputObject());
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetBlock(LavDevice* device, float* destination) {
	LOCK(*device);
	device->getBlock(destination);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetBlockSize(LavDevice* dev, int* destination) {
	PUB_BEGIN
	LOCK(*dev);
	*destination = dev->getBlockSize();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetSr(LavDevice* device, int* destination) {
	PUB_BEGIN
	LOCK(*device);
	*destination = (int)device->getSr();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetChannels(LavDevice* device, int* destination) {
	PUB_BEGIN
	LOCK(*device);
	*destination = device->getChannels();
	PUB_END
}
