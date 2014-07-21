/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_callbacks.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <string>
#include <memory>

/**Callbacks.*/
void LavCallback::setHandler(LavEventCallback cb) {
	handler = cb;
}

LavEventCallback LavCallback::getHandler() {
	return handler;
}

void LavCallback::fire() {
	if(handler == nullptr) return; //nothing to do.
	//we need to hold local copies of both the object and data in case they are changed between firing and processing by the device.
	auto obj = associated_object->shared_from_this();
	void* userdata = user_data;
	LavEventCallback cb = handler;
	//fire a lambda that uses these by copy.
	associated_device->enqueueTask([=]() {
		cb(obj.get(), userdata);
	});
}

void LavCallback::associateDevice(std::shared_ptr<LavDevice> dev) {
	associated_device = dev;
}

void LavCallback::associateObject(LavObject* obj) {
	associated_object = obj;
}

const char* LavCallback::getName() {
	return name.c_str();
}

void LavCallback::setName(const char* n) {
	name = std::string(n);
}

void* LavCallback::getUserData() {
	return user_data;
}

void LavCallback::setUserData(void* data) {
	user_data = data;
}
