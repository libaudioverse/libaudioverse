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
#include <libaudioverse/private_memory.hpp>
#include <limits>
#include <memory>

class LavFileObject: public LavObject {
	LavFileObject(LavDevice* device, const char* path, unsigned int channels);
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

//the third parameter is a hint: we need to know how many channels, we only expose objects through the create functions, so the create function can find this out.
//todo: when objects support resizing their inputs and outputs, as they will inevitably support this, rewrite to use that functionality.
LavFileObject::LavFileObject(std::shared_ptr<LavDevice> device, const char* path, unsigned int channels): LavObject(Lav_OBJTYPE_FILE, device, 0, channels) {
	file.open(path);
	buffer = new float[file.getSampleCount()];
	file.readAll(buffer);
	delta = file.getSr()/device->getSr();
	getProperty(Lav_FILE_POSITION).setPostChangedCallback([this] () {seek();});
	getProperty(Lav_FILE_POSITION).setFloatRange(0.0f, file.getFrameCount()/(float)file.getSr());
}

std::shared_ptr<LavObject>createFileObject(std::shared_ptr<LavDevice> device, const char* path) {
	auto f = LavFileReader();
	f.open(path);
	auto retval = std::make_shared<LavFileObject>(device, path, f.getChannelCount());
	device->associateObject(retval);
	return retval;
}

void LavFileObject::seek() {
	float pos = getProperty(Lav_FILE_POSITION).getFloatValue();
	offset = 0.0f;
	position = (unsigned int)(pos*file.getSr());
}

void LavFileObject::process() {
	const float pitch_bend = getProperty(Lav_FILE_PITCH_BEND).getFloatValue();
	for(unsigned int i = 0; i < block_size; i++) {
		if(offset >= file.getFrameCount()) {
			for(unsigned int j = 0; j < num_outputs; j++) {
				outputs[j][i] = 0.0f;
			}
			continue;
		}
		const unsigned int samp1 = (unsigned int)position;
		const unsigned int samp2 = (unsigned int)position+1;
		const float weight1 = 1-offset;
		const float weight2 = offset;
		for(unsigned int j = 0; j < num_outputs; j++) {
			const unsigned int ind1 = samp1*num_outputs+j;
			const unsigned int ind2 = samp2*num_outputs+j;
			outputs[j][i] = weight1*buffer[ind1]+weight2*buffer[ind2];
		}		
	offset += delta*pitch_bend;
	position += (unsigned int)offset;
	offset = ringmodf(offset, 1.0f);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createFileObject(LavDevice* device, const char* path, LavObject** destination) {
	PUB_BEGIN
	LOCK(*device);
	auto retval = createFileObject(device, path);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
