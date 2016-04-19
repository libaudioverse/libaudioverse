/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/

#include <libaudioverse/3d/source.hpp>
#include <libaudioverse/3d/environment.hpp>
#include <libaudioverse/implementations/amplitude_panner.hpp>
#include <libaudioverse/implementations/multipanner.hpp>
#include <libaudioverse/implementations/biquad.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/workspace.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <libaudioverse/private/error.hpp>
#include <math.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <vector>
#include <map>

namespace libaudioverse_implementation {

//This workspace is used as a temporary buffer.
//Rather than keep this with each source, we make it thread_local. This helps with cache friendliness.
//primarily this is occlusion.
thread_local Workspace<float> source_workspace;

SourceNode::SourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment): Node(Lav_OBJTYPE_SOURCE_NODE, simulation, 1, 0),
hrtf_panner(simulation->getBlockSize(), simulation->getSr(), environment->getHrtf()),
stereo_panner(simulation->getBlockSize(), simulation->getSr()),
surround40_panner(simulation->getBlockSize(), simulation->getSr()),
surround51_panner(simulation->getBlockSize(), simulation->getSr()),
surround71_panner(simulation->getBlockSize(), simulation->getSr()),
occlusion_filter(simulation->getSr()),
hrtf_data(environment->getHrtf()) {
	this->environment = environment;
	handleOcclusion(); //Make sure we initialize as unoccluded.	
	//we have to read off these defaults manually, and it must always be the last thing in the constructor.
	getProperty(Lav_SOURCE_MAX_DISTANCE).setFloatValue(environment->getProperty(Lav_ENVIRONMENT_DEFAULT_MAX_DISTANCE).getFloatValue());
	getProperty(Lav_SOURCE_SIZE).setFloatValue(environment->getProperty(Lav_ENVIRONMENT_DEFAULT_SIZE).getFloatValue());
	getProperty(Lav_SOURCE_REVERB_DISTANCE).setFloatValue(environment->getProperty(Lav_ENVIRONMENT_DEFAULT_REVERB_DISTANCE).getFloatValue());
	appendInputConnection(0, 1);
	stereo_panner.readMap(2, standard_panning_map_stereo);
	surround40_panner.readMap(4, standard_panning_map_surround40);
	surround51_panner.readMap(6, standard_panning_map_surround51);
	surround71_panner.readMap(8, standard_panning_map_surround71);
}

SourceNode::~SourceNode() {
}

std::shared_ptr<SourceNode> createSourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment) {
	auto ret = standardNodeCreation<SourceNode>(simulation, environment);
	environment->registerSourceForUpdates(ret);
	return ret;
}

void SourceNode::feedEffect(int which) {
	if(which < 0 || which >= environment->getEffectSendCount()) ERROR(Lav_ERROR_RANGE, "Invalid effect send.");
	if(fed_effects.count(which)) return; //no-op.
	AmplitudePanner* p;
	int c = environment->getEffectSend(which).channels;
	if(c == 1) p = nullptr;
	else if(c == 2) p = &stereo_panner;
	else if(c == 4) p = &surround40_panner;
	else if(c == 6) p = &surround51_panner;
	else if(c == 8) p = &surround71_panner;
	else ERROR(Lav_ERROR_INTERNAL, "Got invalid effect send count somehow.");
	fed_effects[which] = p;
}

void SourceNode::stopFeedingEffect(int which) {
	if(which < 0 || which >= environment->getEffectSendCount()) ERROR(Lav_ERROR_RANGE, "Invalid effect send.");
	if(fed_effects.count(which)) fed_effects.erase(which);
}

void SourceNode::reset() {
}

//helper function: calculates gains given distance models.
float calculateGainForDistanceModel(int model, float distance, float maxDistance, float referenceDistance) {
	float retval = 1.0f;
	float adjustedDistance = std::max<float>(0.0f, distance-referenceDistance);
		if(adjustedDistance > maxDistance) {
		retval = 0.0f;
	}
	else {
		switch(model) {
			case Lav_DISTANCE_MODEL_LINEAR: retval = 1.0f-(adjustedDistance/maxDistance); break;
			case Lav_DISTANCE_MODEL_EXPONENTIAL: retval = 1.0f/adjustedDistance; break;
			case Lav_DISTANCE_MODEL_INVERSE_SQUARE: retval = 1.0f/(adjustedDistance*adjustedDistance); break;
		}
	}

	//safety clamping.  Some of the equations above will go negative after max_distance.
	if(retval < 0.0f) retval = 0.0f;
	return retval;
}

int SourceNode::computeDistanceModel(EnvironmentInfo& env) {
	int dm = getProperty(Lav_SOURCE_DISTANCE_MODEL).getIntValue();
	if(dm == Lav_DISTANCE_MODEL_DELEGATE) dm = env.distance_model;
	return dm;
}

void SourceNode::update(EnvironmentInfo &env) {
	//first, extract the vector of our position.
	const float* pos = getProperty(Lav_3D_POSITION).getFloat3Value();
	bool isHeadRelative = getProperty(Lav_SOURCE_HEAD_RELATIVE).getIntValue() == 1;
	glm::vec4 npos;
	if(isHeadRelative) npos = glm::vec4(pos[0], pos[1], pos[2], 1.0);
	else npos = env.world_to_listener_transform*glm::vec4(pos[0], pos[1], pos[2], 1.0f);
	//npos is now easy to work with.
	float distance = glm::length(npos);
	float maxDistance = getProperty(Lav_SOURCE_MAX_DISTANCE).getFloatValue();
	//Decide if we're culled. if we are, bale out now and mark us as such.
	if(distance > maxDistance) {
		culled = true;
		return;
	}
	else culled = false;
	float xz = sqrtf(npos.x*npos.x+npos.z*npos.z);
	//elevation and azimuth, in degrees.
	float elevation = atan2f(npos.y, xz)/PI*180.0f;
	float azimuth = atan2(npos.x, -npos.z)/PI*180.0f;
	//Elevation can be slightly over or under due to floating point error.
	//This would trigger an exception because elevation is a property with a range.
	if(elevation > 90.0f) elevation = 90.0f;
	if(elevation < -90.0f) elevation = -90.0f;
	int distanceModel = computeDistanceModel(env);
	float referenceDistance = getProperty(Lav_SOURCE_SIZE).getFloatValue();
	float reverbDistance = getProperty(Lav_SOURCE_REVERB_DISTANCE).getFloatValue();
	dry_gain = calculateGainForDistanceModel(distanceModel, distance, maxDistance, referenceDistance);
	float unscaledReverbMultiplier = 1.0f-calculateGainForDistanceModel(distanceModel, distance, reverbDistance, 0.0f);
	float minReverbLevel = getProperty(Lav_SOURCE_MIN_REVERB_LEVEL).getFloatValue();
	float maxReverbLevel = getProperty(Lav_SOURCE_MAX_REVERB_LEVEL).getFloatValue();
	float scaledReverbMultiplier = minReverbLevel+(maxReverbLevel-minReverbLevel)*unscaledReverbMultiplier;
	reverb_gain = dry_gain*scaledReverbMultiplier;
	int reverbCount = 0;
	for(auto s: fed_effects) reverbCount += environment->getEffectSend(s.first).is_reverb;
	//The logic here is that this is the average gain for all the diffuse field.
	if(reverbCount) reverb_gain /= reverbCount;
	//Bring in mul.
	float mul = getProperty(Lav_NODE_MUL).getFloatValue();
	dry_gain*=mul;
	reverb_gain*=mul;
	//Apply these.
	hrtf_panner.setAzimuth(azimuth);
	hrtf_panner.setElevation(elevation);
	stereo_panner.setAzimuth(azimuth);
	stereo_panner.setElevation(elevation);
	surround40_panner.setAzimuth(azimuth);
	surround40_panner.setElevation(elevation);
	surround51_panner.setAzimuth(azimuth);
	surround51_panner.setElevation(elevation);
	surround71_panner.setAzimuth(azimuth);
	surround71_panner.setElevation(elevation);
	handleOcclusion();
	panning_strategy = getProperty(Lav_SOURCE_PANNING_STRATEGY).getIntValue();
	if(panning_strategy == Lav_PANNING_STRATEGY_DELEGATE) panning_strategy = env.panning_strategy;
}

void SourceNode::process() {
	if(culled) return; //nothing to do.
	//The panners always skip channels 2 and 3, so 7.1 only needs 6 pointers. Plus 1 for the occlusion.
	//We put these together because this increases cache locality.
	float* ws = source_workspace.get(block_size*7);
	float* occluded = ws;
	//The two nullptrs are never, ever used by panners. Ever.
	float* panBuffers[] = {ws+block_size, ws+2*block_size, nullptr, nullptr, ws+3*block_size, ws+4*block_size, ws+5*block_size, ws+6*block_size};
	for(int i = 0; i < block_size; i++) occluded[i] = occlusion_filter.tick(input_buffers[0][i]);

	int channels = 0;
	//The following could be replaced with a multipanner.
	//if we did that, however, we'd have some extra, unavoidable copies.  So we don't.
	switch(panning_strategy) {
		case Lav_PANNING_STRATEGY_HRTF:
		hrtf_panner.pan(occluded, panBuffers[0], panBuffers[1]);
		channels = 2;
		break;
		case Lav_PANNING_STRATEGY_STEREO:
		stereo_panner.pan(occluded, panBuffers);
		channels = 2;
		break;
		case Lav_PANNING_STRATEGY_SURROUND40:
		surround40_panner.pan(occluded, panBuffers);
		channels = 4;
		break;
		case Lav_PANNING_STRATEGY_SURROUND51:
		surround51_panner.pan(occluded, panBuffers);
		channels = 6;
		break;
		case Lav_PANNING_STRATEGY_SURROUND71:
		surround71_panner.pan(occluded, panBuffers);
		channels = 8;
		break;
	}
	for(int i = 0; i < channels; i++) if(panBuffers[i]) multiplicationAdditionKernel(block_size, dry_gain, panBuffers[i], environment->source_buffers[i], environment->source_buffers[i]);
	for(auto &s: fed_effects) {
		auto &send = environment->getEffectSend(s.first);
		auto &p = s.second;
		float g = send.is_reverb ? reverb_gain : dry_gain;
		if(send.channels == 1) multiplicationAdditionKernel(block_size, g, occluded, environment->source_buffers[send.start], environment->source_buffers[send.start]);
		else {
			p->pan(occluded, panBuffers);
			for(int i = 0; i < send.channels; i++) if(panBuffers[i]) multiplicationAdditionKernel(block_size, g, panBuffers[i], environment->source_buffers[send.start+i], environment->source_buffers[send.start+i]);
		}
	}
}

void 	SourceNode::handleOcclusion() {
	//We need a db gain and a frequency from the linear occlusion value.
	float occlusionPercent = getProperty(Lav_SOURCE_OCCLUSION).getFloatValue();
	if(occlusionPercent == 0.0f) {
		//Configure as wire and return.
		//We can go back and forth from any filter type to identity without a problem; this is safe.
		occlusion_filter.configure(Lav_BIQUAD_TYPE_IDENTITY, 0.0f, 0.0f, 0.0f);
		return;
	}
	//-70 DB is fully occluded.
	float dbgain = occlusionPercent*-70.0f;
	//We get the frequency via an exponential function, so that occlusion sounds roughly linear.
	//We scale this to be on the range 200 to 800.
	//It appears that E isn't in math.h on all compilers,  so we use exp(1) for it.
	//Note: e^0 is 1, e^1 is e.
	float frequencyScaleFactor = 1000.0/exp(1);
	//Note: 0 must be furthest away from the origin, unlike frequency.
	float scaledFrequency = frequencyScaleFactor*exp(1-occlusionPercent);
	occlusion_filter.configure(Lav_BIQUAD_TYPE_HIGHSHELF, scaledFrequency, dbgain, 0.5);
}

//Begin public API.

Lav_PUBLIC_FUNCTION LavError Lav_createSourceNode(LavHandle simulationHandle, LavHandle environmentHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createSourceNode(simulation, incomingObject<EnvironmentNode>(environmentHandle));
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_sourceNodeFeedEffect(LavHandle nodeHandle, int effect) {
	PUB_BEGIN
	auto s = incomingObject<SourceNode>(nodeHandle);
	LOCK(*s);
	//Note that external indexes are 1-based.
	s->feedEffect(effect-1);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_sourceNodeStopFeedingEffect(LavHandle nodeHandle, int effect) {
	PUB_BEGIN
	auto s = incomingObject<SourceNode>(nodeHandle);
	LOCK(*s);
	//Note that external indexes are 1-based.
	s->stopFeedingEffect(effect-1);
	PUB_END
}

}