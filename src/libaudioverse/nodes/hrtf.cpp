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
#include <libaudioverse/implementations/delayline.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/constants.hpp>
#include <limits>
#include <functional>
#include <algorithm>
#include <memory>
#include <math.h>

namespace libaudioverse_implementation {

class HrtfNode: public Node {
	public:
	HrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
	~HrtfNode();
	virtual void process();
	void reset();
	//the difference between the time the sound would reach the left ear and the time it would reach the right.
	//returns positive values if the right ear is greater, negative if the left ear is greater.
	float computeInterauralDelay();
	private:
	int history_length = 0;
	float *history = nullptr, *left_response = nullptr, *right_response = nullptr, *old_left_response = nullptr, *old_right_response = nullptr;
	std::shared_ptr<HrtfData> hrtf = nullptr;
	float prev_azimuth = 0.0f, prev_elevation = 0.0f;
	//variables for the interaural time difference.
	CrossfadingDelayLine left_delay_line, right_delay_line;
	const float max_interaural_delay = 0.02;
};

HrtfNode::HrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf): Node(Lav_OBJTYPE_HRTF_NODE, simulation, 1, 2),
left_delay_line(0.02, simulation->getSr()),
right_delay_line(0.02, simulation->getSr()) {
	type = Lav_OBJTYPE_HRTF_NODE;
	this->hrtf = hrtf;
	left_response = allocArray<float>(hrtf->getLength()*sizeof(float));
	right_response = allocArray<float>(hrtf->getLength()*sizeof(float));
	//used for moving objects.
	old_left_response = allocArray<float>(hrtf->getLength());
	old_right_response = allocArray<float>(hrtf->getLength());
	history_length = hrtf->getLength() + simulation->getBlockSize();
	history = allocArray<float>(history_length);
	hrtf->computeCoefficientsStereo(0.0f, 0.0f, left_response, right_response);
	prev_azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	prev_elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	appendInputConnection(0, 1);
	appendOutputConnection(0, 2);
	//the crossffading time of both delay lines should be one block.
	left_delay_line.setInterpolationTime(simulation->getBlockSize()/simulation->getSr());
	right_delay_line.setInterpolationTime(simulation->getBlockSize()/simulation->getSr());
}

HrtfNode::~HrtfNode() {
	freeArray(history);
	freeArray(left_response);
	freeArray(right_response);
	freeArray(old_left_response);
	freeArray(old_right_response);
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
	//we compute the interaural delay and apply it to the lines.
	float interauralDelay = computeInterauralDelay();
	if(interauralDelay > 0) {
		left_delay_line.setDelay(0.0);
		right_delay_line.setDelay(std::min(interauralDelay, max_interaural_delay));
	}
	else {
		left_delay_line.setDelay(std::min(-interauralDelay, max_interaural_delay));
		right_delay_line.setDelay(0.0);
	}
	//apply the delay lines by ticking over our outputs.
	for(int i=0; i < block_size; i++) output_buffers[0][i] = left_delay_line.tick(output_buffers[0][i]);
	for(int i=0; i < block_size; i++) output_buffers[1][i] = right_delay_line.tick(output_buffers[1][i]);
}

float HrtfNode::computeInterauralDelay() {
	float headWidth=getProperty(Lav_PANNER_HEAD_WIDTH).getFloatValue();
	if(headWidth==0.0f) return 0.0f; //Heads of no width can't have interaural delays.
	float speedOfSound = getProperty(Lav_PANNER_SPEED_OF_SOUND).getFloatValue();
	float distance=getProperty(Lav_PANNER_DISTANCE).getFloatValue();
	float headRadius = headWidth/2;
	float earPosition = getProperty(Lav_PANNER_EAR_POSITION).getFloatValue();
	earPosition *= -headRadius;
	distance=std::max(distance, headRadius);
	//take these to radians immediately.
	float azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue()*PI/180.0f;
	float elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue()*PI/180.0f;
	//first, compute the point of the sound.
	//s for sound.
	float sx, sy, sz;
	float xyPlane = distance*cosf(elevation);
	sx = xyPlane*sinf(azimuth);
	sy = xyPlane*cosf(azimuth);
	sz = distance*sinf(elevation);
	//apply pythagorean theorem.
	float leftDistance = sqrtf((sx-(-headRadius))*(sx-(-headRadius))+(sy-earPosition)*(sy-earPosition)+sz*sz);
	float rightDistance = sqrtf((sx-headRadius)*(sx-headRadius)+(sy-earPosition)*(sy-earPosition)+sz*sz);
	//the order of the subtraction here makes us return positive when right ear has greater delay.
	float delta = rightDistance-leftDistance;
	//Finally, divide by speed of sound and return.
	return delta/speedOfSound;
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

}