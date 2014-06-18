/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <stdlib.h>
#include <functional>
#include <algorithm>
#include <iterator>

LavDevice::LavDevice(unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixahead) {
	this->sr = (float)sr;
	this->channels = channels;
	this->block_size = blockSize;
	this->mixahead = mixahead;
}

LavError LavDevice::getBlock(float* out) {
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
	for(unsigned int i = 0; i < obj->getParentCount(); i++) {
		visitAllObjectsReachableFrom(obj->getParentObject(i), visitor);
	}
	visitor(obj);
}
