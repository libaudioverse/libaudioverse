/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_simulation.hpp>
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
	LavHrtfObject(std::shared_ptr<LavSimulation> simulation, std::shared_ptr<LavHrtfData> hrtf);
	~LavHrtfObject();
	virtual void process();
	void reset();
	private:
	int history_length = 0;
	float *history = nullptr, *left_response = nullptr, *right_response = nullptr, *old_left_response = nullptr, *old_right_response = nullptr;
	std::shared_ptr<LavHrtfData> hrtf = nullptr;
	float prev_azimuth = 0.0f, prev_elevation = 0.0f;
};

LavHrtfObject::LavHrtfObject(std::shared_ptr<LavSimulation> simulation, std::shared_ptr<LavHrtfData> hrtf): LavObject(Lav_OBJTYPE_HRTF, simulation, 1, 2) {
	type = Lav_OBJTYPE_HRTF;
	this->hrtf = hrtf;
	left_response = LavAllocFloatArray(hrtf->getLength()*sizeof(float));
	right_response = LavAllocFloatArray(hrtf->getLength()*sizeof(float));
	//used for moving objects.
	old_left_response = LavAllocFloatArray(hrtf->getLength());
	old_right_response = LavAllocFloatArray(hrtf->getLength());
	history_length = hrtf->getLength() + simulation->getBlockSize();
	history = LavAllocFloatArray(history_length);
	hrtf->computeCoefficientsStereo(0.0f, 0.0f, left_response, right_response);
	prev_azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	prev_elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
}

LavHrtfObject::~LavHrtfObject() {
	LavFreeFloatArray(history);
	LavFreeFloatArray(left_response);
	LavFreeFloatArray(right_response);
	LavFreeFloatArray(old_left_response);
	LavFreeFloatArray(old_right_response);
}

std::shared_ptr<LavObject>createHrtfObject(std::shared_ptr<LavSimulation>simulation, std::shared_ptr<LavHrtfData> hrtf) {
	auto retval = std::shared_ptr<LavHrtfObject>(new LavHrtfObject(simulation, hrtf), LavObjectDeleter);
	simulation->associateObject(retval);
	return retval;
}

void LavHrtfObject::process() {
	//calculating the hrir is expensive, do it only if needed.
	bool didRecompute = false;
	bool allowCrossfade = getProperty(Lav_PANNER_SHOULD_CROSSFADE).getIntValue();
	float current_azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	float current_elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	if(fabs(current_elevation-prev_elevation) > 2.0f || fabs(current_azimuth-prev_azimuth) > 2.0f) {
		if(allowCrossfade) {
			std::copy(left_response, left_response+hrtf->getLength(), old_left_response);
			std::copy(right_response, right_response+hrtf->getLength(), old_right_response);
		}
		hrtf->computeCoefficientsStereo(current_elevation, current_azimuth, left_response, right_response);
		didRecompute = true;
	}
	float *start = history+hrtf->getLength(), *end = history+hrtf->getLength()+simulation->getBlockSize();
	//get the block size.
	const unsigned int hrtfLength = hrtf->getLength();
	//roll back the history...
	std::copy(end-hrtf->getLength(), end, history);
	//stick our input on the end...
	std::copy(inputs[0], inputs[0]+block_size, start);
	//finally, do the usual convolution loop.
	if(didRecompute) {
		if(allowCrossfade) {
			crossfadeConvolutionKernel(history, block_size, outputs[0], hrtfLength, old_left_response, left_response);
			crossfadeConvolutionKernel(history, block_size, outputs[1], hrtfLength, old_right_response, right_response);
		}
		else {
			convolutionKernel(history, block_size, outputs[0], hrtf->getLength(), left_response);
			convolutionKernel(history, block_size, outputs[1], hrtf->getLength(), right_response);
		}
		//note: putting these anywhere in the didnt-recompute path causes things to never move.
		prev_elevation = current_elevation;
		prev_azimuth = current_azimuth;
	}
	else {
		convolutionKernel(history, block_size, outputs[0], hrtf->getLength(), left_response);
		convolutionKernel(history, block_size, outputs[1], hrtf->getLength(), right_response);
	}
}

void LavHrtfObject::reset() {
	memset(history, 0, sizeof(float)*history_length);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfObject(LavSimulation* simulation, const char* hrtfPath, LavObject** destination) {
	PUB_BEGIN
	auto hrtf = std::make_shared<LavHrtfData>();
	if(std::string(hrtfPath) != "default") {
		hrtf->loadFromFile(hrtfPath, simulation->getSr());
	} else {
		hrtf->loadFromDefault(simulation->getSr());
	}
	LOCK(*simulation);
	auto retval = createHrtfObject(incomingPointer<LavSimulation>(simulation), hrtf);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
