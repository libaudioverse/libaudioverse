/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <limits>
#include <functional>
#include <algorithm>
#include <memory>
#include <math.h>

class HrtfNode: public Node {
	public:
	HrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
	~HrtfNode();
	virtual void process();
	void reset();
	private:
	int history_length = 0;
	float *history = nullptr, *left_response = nullptr, *right_response = nullptr, *old_left_response = nullptr, *old_right_response = nullptr;
	std::shared_ptr<HrtfData> hrtf = nullptr;
	float prev_azimuth = 0.0f, prev_elevation = 0.0f;
};

HrtfNode::HrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf): Node(Lav_OBJTYPE_HRTF_NODE, simulation, 1, 2) {
	type = Lav_OBJTYPE_HRTF_NODE;
	this->hrtf = hrtf;
	left_response = AllocArray<float>(hrtf->getLength()*sizeof(float));
	right_response = AllocArray<float>(hrtf->getLength()*sizeof(float));
	//used for moving objects.
	old_left_response = AllocArray<float>(hrtf->getLength());
	old_right_response = AllocArray<float>(hrtf->getLength());
	history_length = hrtf->getLength() + simulation->getBlockSize();
	history = AllocArray<float>(history_length);
	hrtf->computeCoefficientsStereo(0.0f, 0.0f, left_response, right_response);
	prev_azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	prev_elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	appendInputConnection(0, 1);
	appendOutputConnection(0, 2);
}

HrtfNode::~HrtfNode() {
	FreeArray(history);
	FreeArray(left_response);
	FreeArray(right_response);
	FreeArray(old_left_response);
	FreeArray(old_right_response);
}

std::shared_ptr<Node>createHrtfNode(std::shared_ptr<Simulation>simulation, std::shared_ptr<HrtfData> hrtf) {
	auto retval = std::shared_ptr<HrtfNode>(new HrtfNode(simulation, hrtf), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void HrtfNode::process() {
	//calculating the hrir is expensive, do it only if needed.
	bool didRecompute = false;
	bool allowCrossfade = getProperty(Lav_PANNER_SHOULD_CROSSFADE).getIntValue();
	float current_azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	float current_elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	if(fabs(current_elevation-prev_elevation) > 0.5f || fabs(current_azimuth-prev_azimuth) > 0.5f) {
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
	std::copy(input_buffers[0], input_buffers[0]+block_size, start);
	//finally, do the usual convolution loop.
	if(didRecompute) {
		if(allowCrossfade) {
			crossfadeConvolutionKernel(history, block_size, output_buffers[0], hrtfLength, old_left_response, left_response);
			crossfadeConvolutionKernel(history, block_size, output_buffers[1], hrtfLength, old_right_response, right_response);
		}
		else {
			convolutionKernel(history, block_size, output_buffers[0], hrtf->getLength(), left_response);
			convolutionKernel(history, block_size, output_buffers[1], hrtf->getLength(), right_response);
		}
		//note: putting these anywhere in the didnt-recompute path causes things to never move.
		prev_elevation = current_elevation;
		prev_azimuth = current_azimuth;
	}
	else {
		convolutionKernel(history, block_size, output_buffers[0], hrtf->getLength(), left_response);
		convolutionKernel(history, block_size, output_buffers[1], hrtf->getLength(), right_response);
	}
}

void HrtfNode::reset() {
	memset(history, 0, sizeof(float)*history_length);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfNode(LavHandle simulationHandle, const char* hrtfPath, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto hrtf = std::make_shared<HrtfData>();
	if(std::string(hrtfPath) != "default") {
		hrtf->loadFromFile(hrtfPath, simulation->getSr());
	} else {
		hrtf->loadFromDefault(simulation->getSr());
	}
	auto retval = createHrtfNode(simulation, hrtf);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}
