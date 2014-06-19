/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_macros.hpp>
#include <stdlib.h>
#include <functional>
#include <algorithm>
#include <iterator>

void LavDevice::init(unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixahead) {
	this->sr = (float)sr;
	this->channels = channels;
	this->block_size = blockSize;
	this->mixahead = mixahead;
}

LavError LavDevice::getBlock(float* out) {
	//if paused, memset 0s.
	if(is_started == 0) {
		memset(out, 0, sizeof(float)*channels*block_size);
		return Lav_ERROR_NONE;
	}
	//okay, we're not paused.  Visit all objects, calling their process method.
	visitAllObjectsInProcessOrder([] (LavObject* o) {o->process();});
	//if no output object, memset 0 and bail out again.
	if(output_object == nullptr) {
		memset(out, 0, sizeof(float)*channels*block_size);
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

LavError LavDevice::start() {
	is_started = 1;
	return Lav_ERROR_NONE;
}

LavError LavDevice::stop() {
	is_started = 0;
	return Lav_ERROR_NONE;
}

LavError LavDevice::associateObject(LavObject* obj) {
	objects.insert(obj);
	return Lav_ERROR_NONE;
}

LavError LavDevice::setOutputObject(LavObject* obj) {
	output_object = obj;
	return Lav_ERROR_NONE;
}

LavObject* LavDevice::getOutputObject() {
	return output_object;
}

void LavDevice::visitAllObjectsInProcessOrder(std::function<void(LavObject*)> visitor) {
	std::set<LavObject*> seen;
	if(output_object) {
		visitAllObjectsReachableFrom(output_object, [&](LavObject* o) {
			visitor(o);
			seen.insert(o);
		});
	}
	std::set<LavObject*> still_needed;
	do {
		still_needed.clear();
		std::set_difference(seen.begin(), seen.end(), always_process.begin(), always_process.end(),
			std::inserter(still_needed, still_needed.end()));
		for(auto i = still_needed.begin(); i != still_needed.end(); i++) {
			visitAllObjectsReachableFrom(*i, [&] (LavObject* o) {
				if(seen.count(o) == 0) {
					visitor(o);
					seen.insert(o);
				}
			});
		}
	} while(still_needed.size());
}

void LavDevice::visitAllObjectsReachableFrom(LavObject* obj, std::function<void(LavObject*)> visitor) {
	//we call ourselves on all parents of obj, and then pass obj to visitor.  This is essentially depth-first search.
	for(unsigned int i = 0; i < obj->getInputCount(); i++) {
		visitAllObjectsReachableFrom(obj->getParentObject(i), visitor);
	}
	visitor(obj);
}

//begin public API

Lav_PUBLIC_FUNCTION LavError Lav_deviceSetOutputObject(LavDevice* device, LavObject* object) {
	LOCK(*device);
	device->setOutputObject(object);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetOutputObject(LavDevice* device, LavObject** destination) {
	LOCK(*device);
	*destination = device->getOutputObject();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_deviceGetBlock(LavDevice* device, float* destination) {
	LOCK(*device);
	device->getBlock(destination);
	return Lav_ERROR_NONE;
}
