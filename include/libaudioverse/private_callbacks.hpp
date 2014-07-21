/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include <utility>
#include <memory>
#include <string>

class LavDevice;

class LavCallback {
	public:
	void setHandler(LavEventCallback cb);
	LavEventCallback getHandler();
	void fire();
	void associateDevice(std::shared_ptr<LavDevice> dev);
	void associateObject(LavObject* obj);
	const char* getName();
	void setName(const char* n);
	void* getUserData();
	void setUserData(void* data);
	private:
	std::shared_ptr<LavDevice> associated_device = nullptr;
	LavEventCallback handler = nullptr;
	std::string name;
	LavObject* associated_object;
	void* user_data;
};