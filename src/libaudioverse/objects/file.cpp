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
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/private_properties.hpp>
#include <limits>

class LavFileObject: public LavObject {
	virtual void init(LavDevice* device, const char* path);
	virtual void process();
	void seek(); //property callback.
	protected:
	LavFileReader file;
	float* buffer = nullptr;
	unsigned int position = 0;
	float offset = 0;
	float delta = 0.0f;
	friend LavObject* createFileObject(LavDevice* device, const char* path);
};

void LavFileObject::init(LavDevice* device, const char* path) {
	LavError err = file.open(path);
	if(err != Lav_ERROR_NONE) return;
	LavObject::init(device, 0, file.getChannelCount());
	buffer = new float[file.getSampleCount()];
	file.readAll(buffer);
	delta = file.getSr()/device->getSr();
	properties[Lav_FILE_POSITION] = createFloatProperty("position", 0.0f, 0.0f, file.getFrameCount()/file.getSr());
	properties[Lav_FILE_PITCH_BEND] = createFloatProperty("pitch_bend", 1.0f, 0.0f, std::numeric_limits<float>::infinity());
	properties[Lav_FILE_POSITION]->setPostChangedCallback([&] () {seek();});
}

LavObject* createFileObject(LavDevice* device, const char* path) {
	auto retval = new LavFileObject();
	retval->init(device, path);
	return retval;
}

void LavFileObject::seek() {
	float pos = properties[Lav_FILE_POSITION]->getFloatValue();
	offset = 0.0f;
	position = (unsigned int)(pos*file.getSr());
}

void LavFileObject::process() {
	const float pitch_bend = properties[Lav_FILE_PITCH_BEND]->getFloatValue();
	for(unsigned int i = 0; i < device->getBlockSize(); i++) {
		if(offset >= file.getFrameCount()) {
			for(unsigned int j = 0; j < num_outputs; j++) {
				outputs[j][i] = 0.0f;
			}
			continue;
		}
		unsigned int samp1 = (unsigned int)position;
		unsigned int samp2 = (unsigned int)position+1;
		float weight1 = 1-offset;
		float weight2 = offset;
		for(unsigned int j = 0; j < num_outputs; j++) {
			unsigned int ind1 = samp1*num_outputs+j;
			unsigned int ind2 = samp2*num_outputs+j;
			outputs[j][i] = weight1*buffer[ind1]+weight2*buffer[ind2];
		}		
	offset += delta*pitch_bend;
	position += (unsigned int)offset;
	offset = ringmodf(offset, 1.0f);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createFileObject(LavDevice* device, const char* path, LavObject** destination) {
	LOCK(*device);
	auto retval = createFileObject(device, path);
	*destination = retval;
	return Lav_ERROR_NONE;
}
