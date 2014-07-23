/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_hrtf.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_kernels.hpp>
#include <limits>
#include <functional>
#include <algorithm>
#include <memory>
#include <math.h>

class LavHrtfObject: public LavObject {
	public:
	LavHrtfObject(std::shared_ptr<LavDevice> device, std::shared_ptr<LavHrtfData> hrtf);
	~LavHrtfObject();
	virtual void process();
	private:
	float *history = nullptr, *left_response = nullptr, *right_response = nullptr, *old_left_response = nullptr, *old_right_response = nullptr;
	std::shared_ptr<LavHrtfData> hrtf = nullptr;
	float prev_azimuth = 0.0f, prev_elevation = 0.0f;
};

LavHrtfObject::LavHrtfObject(std::shared_ptr<LavDevice> device, std::shared_ptr<LavHrtfData> hrtf): LavObject(Lav_OBJTYPE_HRTF, device, 1, 2) {
	type = Lav_OBJTYPE_HRTF;
	this->hrtf = hrtf;
	left_response = new float[hrtf->getLength()];
	right_response = new float[hrtf->getLength()];
	//used for moving objects.
	old_left_response = new float[hrtf->getLength()];
	old_right_response = new float[hrtf->getLength()];
	history = new float[hrtf->getLength() + device->getBlockSize()](); //odd c++ syntax to create 0-initialized array.
	hrtf->computeCoefficientsStereo(0.0f, 0.0f, left_response, right_response);
	prev_azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	prev_elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
}

LavHrtfObject::~LavHrtfObject() {
	delete[] history;
	delete[] left_response;
	delete[] right_response;
}

std::shared_ptr<LavObject>createHrtfObject(std::shared_ptr<LavDevice>device, std::shared_ptr<LavHrtfData> hrtf) {
	auto retval = std::make_shared<LavHrtfObject>(device, hrtf);
	device->associateObject(retval);
	return retval;
}

void LavHrtfObject::process() {
	//calculating the hrir is expensive, do it only if needed.
	bool didRecompute = false;
	float current_azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	float current_elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	if(fabs(current_elevation-prev_elevation) > 2.0f || fabs(current_azimuth-prev_azimuth) > 2.0f) {
		std::copy(left_response, left_response+hrtf->getLength(), old_left_response);
		std::copy(right_response, right_response+hrtf->getLength(), old_right_response);
		hrtf->computeCoefficientsStereo(current_elevation, current_azimuth, left_response, right_response);
		didRecompute = true;
	}
	float *start = history+hrtf->getLength(), *end = history+hrtf->getLength()+device->getBlockSize();
	//get the block size.
	const unsigned int hrtfLength = hrtf->getLength();
	//roll back the history...
	std::copy(end-hrtf->getLength(), end, history);
	//stick our input on the end...
	std::copy(inputs[0], inputs[0]+block_size, start);
	//finally, do the usual convolution loop.
	if(didRecompute) { //very, very slow.
		crossfadeConvolutionKernel(history, block_size, outputs[0], hrtfLength, old_left_response, left_response);
		crossfadeConvolutionKernel(history, block_size, outputs[1], hrtfLength, old_right_response, right_response);
	}
	else {
		convolutionKernel(history, block_size, outputs[0], hrtf->getLength(), left_response);
		convolutionKernel(history, block_size, outputs[1], hrtf->getLength(), right_response);
	}
	prev_elevation = current_elevation;
	prev_azimuth = current_azimuth;
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfObject(LavDevice* device, const char* hrtfPath, LavObject** destination) {
	PUB_BEGIN
	auto hrtf = std::make_shared<LavHrtfData>();
	hrtf->loadFromFile(hrtfPath);
	LOCK(*device);
	auto retval = createHrtfObject(incomingPointer<LavDevice>(device), hrtf);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
