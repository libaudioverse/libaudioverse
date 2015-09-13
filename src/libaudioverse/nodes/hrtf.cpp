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
#include <libaudioverse/implementations/convolvers.hpp>
#include <algorithm>
#include <memory>
#include <math.h>
#include <kiss_fftr.h>

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
	void applyIdtChanged();
	private:
	//the hrtf.
	std::shared_ptr<HrtfData> hrtf = nullptr;
	//for determining when we should and shouldn't crossfade.
	float prev_azimuth = 0.0f, prev_elevation = 0.0f;
	//buffers and length for the convolvers.
	float* left_response, *right_response;
	int response_length;
	//the convolvers themselves.
	FftConvolver *left_convolver, *right_convolver, *new_left_convolver, *new_right_convolver;
	//A delta used in crossfading.
	float crossfade_delta=0.0f;
	float* crossfade_workspace;
	//variables for the interaural time difference.
	CrossfadingDelayLine left_delay_line, right_delay_line;
	const float max_interaural_delay = 0.02f;
	//Used to guarantee that we only compute the fft once.
	kiss_fftr_cfg fft = nullptr;
	float* fft_workspace=nullptr;
	kiss_fft_cpx *input_fft = nullptr;
};

HrtfNode::HrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf): Node(Lav_OBJTYPE_HRTF_NODE, simulation, 1, 2),
left_delay_line(0.02, simulation->getSr()),
right_delay_line(0.02, simulation->getSr()) {
	type = Lav_OBJTYPE_HRTF_NODE;
	this->hrtf = hrtf;
	response_length=hrtf->getLength();
	left_response=allocArray<float>(response_length);
	right_response=allocArray<float>(response_length);
	hrtf->computeCoefficientsStereo(0.0f, 0.0f, left_response, right_response);
	//set up the convolvers.
	left_convolver=new FftConvolver(simulation->getBlockSize());
	right_convolver=new FftConvolver(simulation->getBlockSize());
	new_left_convolver=new FftConvolver(simulation->getBlockSize());
	new_right_convolver=new FftConvolver(simulation->getBlockSize());
	left_convolver->setResponse(response_length, left_response);
	right_convolver->setResponse(response_length, right_response);
	prev_azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	prev_elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	crossfade_delta=1.0f/simulation->getBlockSize();
	crossfade_workspace = allocArray<float>(simulation->getBlockSize());
	appendInputConnection(0, 1);
	appendOutputConnection(0, 2);
	//the crossffading time of both delay lines should be one block.
	left_delay_line.setInterpolationTime(simulation->getBlockSize()/simulation->getSr());
	right_delay_line.setInterpolationTime(simulation->getBlockSize()/simulation->getSr());
	//Set up the fft-related stuff.
	fft_workspace=allocArray<float>(left_convolver->getFftSize());
	input_fft=allocArray<kiss_fft_cpx>(left_convolver->getFftSize());
	fft=kiss_fftr_alloc(left_convolver->getFftSize(), 0, nullptr, nullptr);
}

HrtfNode::~HrtfNode() {
	freeArray(left_response);
	freeArray(right_response);
	freeArray(crossfade_workspace);
	freeArray(input_fft);
	freeArray(fft_workspace);
	kiss_fftr_free(fft);
	delete left_convolver;
	delete right_convolver;
	delete new_left_convolver;
	delete new_right_convolver;
}

std::shared_ptr<Node>createHrtfNode(std::shared_ptr<Simulation>simulation, std::shared_ptr<HrtfData> hrtf) {
	return standardNodeCreation<HrtfNode>(simulation, hrtf);
}

void HrtfNode::process() {
	if(werePropertiesModified(this, Lav_PANNER_APPLY_ITD)) applyIdtChanged();
	bool applyingItd = getProperty(Lav_PANNER_APPLY_ITD).getIntValue() == 1;
	bool linearPhase = getProperty(Lav_PANNER_USE_LINEAR_PHASE).getIntValue() == 1;
	//Get the fft of the input.
	std::copy(input_buffers[0], input_buffers[0]+block_size, fft_workspace);
	kiss_fftr(fft, fft_workspace, input_fft);
	//calculating the hrir is expensive, do it only if needed.
	bool didRecompute = false;
	bool allowCrossfade = getProperty(Lav_PANNER_SHOULD_CROSSFADE).getIntValue();
	float currentAzimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	float currentElevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	if(allowCrossfade && (fabs(currentElevation-prev_elevation) > 0.5f || fabs(currentAzimuth-prev_azimuth) > 0.5f)) {
		hrtf->computeCoefficientsStereo(currentElevation, currentAzimuth, left_response, right_response, linearPhase);
		if(allowCrossfade) {
			new_left_convolver->setResponse(response_length, left_response);
			new_right_convolver->setResponse(response_length, right_response);
		}
		else {
			left_convolver->setResponse(response_length, left_response);
			right_convolver->setResponse(response_length, right_response);
		}
		didRecompute=true;
		//note: putting these anywhere in the didnt-recompute path causes things to never move.
		prev_elevation = currentElevation;
		prev_azimuth = currentAzimuth;
	}
	//These happen regardless of if we are crossfading or recomputed.
	left_convolver->convolveFft(input_fft, output_buffers[0]);
	right_convolver->convolveFft(input_fft, output_buffers[1]);
	//If we crossfaded, we need to apply the following change.
	if(didRecompute && allowCrossfade) {
		//Shape the buffers as follows, enabling a simple add.
		for(int i=0; i < block_size; i++) {
			float w = 1.0f-i*crossfade_delta;
			output_buffers[0][i]*=w;
			output_buffers[1][i] *= w;
		}
		//Run the new convolver for the left channel and crossfade.
		//Then, repeat for the right.
		new_left_convolver->convolveFft(input_fft, crossfade_workspace);
		for(int i =0; i < block_size; i++) output_buffers[0][i] += crossfade_workspace[i]*i*crossfade_delta;
		new_right_convolver->convolveFft(input_fft, crossfade_workspace);
		for(int i=0; i < block_size; i++) output_buffers[1][i] += crossfade_workspace[i]*i*crossfade_delta;
		//Finally, swap them.
		std::swap(left_convolver, new_left_convolver);
		std::swap(right_convolver, new_right_convolver);
	}
	//break out early if we aren't doing interaural delay.
	if(applyingItd == false) return;
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
	//apply the delay lines.
	left_delay_line.processBuffer(block_size, output_buffers[0], output_buffers[0]);
	right_delay_line.processBuffer(block_size, output_buffers[1], output_buffers[1]);
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

void HrtfNode::applyIdtChanged() {
	left_delay_line.reset();
	right_delay_line.reset();
}

void HrtfNode::reset() {
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