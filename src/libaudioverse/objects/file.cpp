/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Reads an entire file into memory and plays it with support for dopler and seeking.*/
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_file.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_macros.hpp>

class LavFileObject: public LavObject {
	virtual void init(LavDevice* device, const char* path);
	protected:
	LavFileReader file;
	float* buffer = nullptr;
	unsigned int position = 0;
	float offset = 0;
	float delta = 0.0f;
};

void LavFileObject::init(LavDevice* device, const char* path) {
	LavError err = file.open(path);
	if(err != Lav_ERROR_NONE) return;
	LavObject::init(device, 0, file.getChannelCount());
	buffer = new float[file.getSampleCount()];
	file.readAll(buffer);
	delta = file.getSr()/device->getSr();
}