/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/3d/sources.hpp>
#include <libaudioverse/3d/environmentbase.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/creators.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <libaudioverse/private/errors.hpp>
#include <math.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <limits>
#include <memory>

namespace libaudioverse_implementation {

SourceNode::SourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentBase> manager): SubgraphNode(Lav_OBJTYPE_SOURCE_NODE, simulation) {
	input = createGainNode(simulation);
	input->resize(1, 1);
	input->appendInputConnection(0, 1);
	input->appendOutputConnection(0, 1);
	panner_node = manager->createPannerNode();
	this->manager = manager;
	getProperty(Lav_SOURCE_PANNER_STRATEGY).setPostChangedCallback([this] () {
		this->panner_node->getProperty(Lav_PANNER_STRATEGY).setIntValue(this->getProperty(Lav_SOURCE_PANNER_STRATEGY).getIntValue());
	});
	//we have to read off these defaults manually, and it must always be the last thing in the constructor.
	getProperty(Lav_SOURCE_DISTANCE_MODEL).setIntValue(manager->getProperty(Lav_ENVIRONMENT_DEFAULT_DISTANCE_MODEL).getIntValue());
	getProperty(Lav_SOURCE_MAX_DISTANCE).setFloatValue(manager->getProperty(Lav_ENVIRONMENT_DEFAULT_MAX_DISTANCE).getFloatValue());
	getProperty(Lav_SOURCE_PANNER_STRATEGY).setIntValue(manager->getProperty(Lav_ENVIRONMENT_DEFAULT_PANNER_STRATEGY).getIntValue());
	getProperty(Lav_SOURCE_SIZE).setFloatValue(manager->getProperty(Lav_ENVIRONMENT_DEFAULT_SIZE).getFloatValue());
	input->connect(0, panner_node, 0);
	setInputNode(input);
}

std::shared_ptr<Node> createSourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentBase> manager) {
	auto temp = std::shared_ptr<SourceNode>(new SourceNode(simulation, manager), ObjectDeleter(simulation));
	manager->registerSourceForUpdates(temp);
	simulation->associateNode(temp);
	return temp;
}

//helper function: calculates gains given distance models.
float calculateGainForDistanceModel(int model, float distance, float maxDistance, float referenceDistance) {
	float retval = 1.0f;
	float adjustedDistance = std::max<float>(0.0f, distance-referenceDistance);
	switch(model) {
		case Lav_DISTANCE_MODEL_LINEAR: retval = 1.0f-(adjustedDistance/maxDistance); break;
		case Lav_DISTANCE_MODEL_EXPONENTIAL: retval = 1.0f/adjustedDistance; break;
		case Lav_DISTANCE_MODEL_INVERSE_SQUARE: retval = 1.0f/(adjustedDistance*adjustedDistance); break;
	}

	//safety clamping.  Some of the equations above will go negative after max_distance.
	if(retval < 0.0f) retval = 0.0f;
	return retval;
}

void SourceNode::update(Environment environment) {
	//first, extract the vector of our position.
	const float* pos = getProperty(Lav_3D_POSITION).getFloat3Value();
	glm::vec4 npos = environment.world_to_listener_transform*glm::vec4(pos[0], pos[1], pos[2], 1.0f);
	//npos is now easy to work with.
	float distance = glm::length(npos);
	float xz = sqrtf(npos.x*npos.x+npos.z*npos.z);
	//elevation and azimuth, in degrees.
	float elevation = atan2f(npos.y, xz)/PI*180.0f;
	float azimuth = atan2(npos.x, -npos.z)/PI*180.0f;
	if(elevation > 90.0f) elevation = 90.0f;
	if(elevation < -90.0f) elevation = -90.0f;
	int distanceModel = getProperty(Lav_SOURCE_DISTANCE_MODEL).getIntValue();
	float maxDistance = getProperty(Lav_SOURCE_MAX_DISTANCE).getFloatValue();
	float referenceDistance = getProperty(Lav_SOURCE_SIZE).getFloatValue();
	float gain = calculateGainForDistanceModel(distanceModel, distance, maxDistance, referenceDistance);
	//Add in our mul.
	gain *= getProperty(Lav_NODE_MUL).getFloatValue();

	//set the panner.
	panner_node->getProperty(Lav_PANNER_AZIMUTH).setFloatValue(azimuth);
	panner_node->getProperty(Lav_PANNER_ELEVATION).setFloatValue(elevation);
	panner_node ->getProperty(Lav_NODE_MUL).setFloatValue(gain);
}

Lav_PUBLIC_FUNCTION LavError Lav_createSourceNode(LavHandle simulationHandle, LavHandle environmentHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createSourceNode(simulation, incomingObject<EnvironmentBase>(environmentHandle));
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}