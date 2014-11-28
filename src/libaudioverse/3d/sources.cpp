/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/3d/private_sources.hpp>
#include <libaudioverse/3d/private_environmentbase.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_constants.hpp>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_creators.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <libaudioverse/private_errors.hpp>
#include <math.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <limits>
#include <memory>


LavSourceObject::LavSourceObject(std::shared_ptr<LavSimulation> simulation, std::shared_ptr<LavEnvironmentBase> manager): LavSubgraphObject(Lav_OBJTYPE_SOURCE, simulation) {
	panner_object = manager->createPannerObject();
	configureSubgraph(panner_object, nullptr); //we input to the panner, and output to nothing (because the manager grabs hold of the panner directly).
	this->manager = manager;
}

std::shared_ptr<LavObject> createSourceObject(std::shared_ptr<LavSimulation> simulation, std::shared_ptr<LavEnvironmentBase> manager) {
	auto temp = std::make_shared<LavSourceObject>(simulation, manager);
	manager->registerSourceForUpdates(temp);
	return temp;
}

//helper function: calculates gains given distance models.
float calculateGainForDistanceModel(int model, float distance, float maxDistance) {
	float retval = 1.0f;
	switch(model) {
		case Lav_DISTANCE_MODEL_LINEAR: retval = 1.0f-(distance/maxDistance); break;
	}

	//safety clamping.  Some of the equations above will go negative after max_distance.
	if(retval < 0.0f) retval = 0.0f;
	return retval;
}

void LavSourceObject::update(LavEnvironment environment) {
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
	float gain = calculateGainForDistanceModel(distanceModel, distance, maxDistance);

	//set the panner.
	panner_object->getProperty(Lav_PANNER_AZIMUTH).setFloatValue(azimuth);
	panner_object->getProperty(Lav_PANNER_ELEVATION).setFloatValue(elevation);
	panner_object ->getProperty(Lav_OBJECT_MUL).setFloatValue(gain);
}

Lav_PUBLIC_FUNCTION LavError Lav_createSourceObject(LavSimulation* simulation, LavObject* environment, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createSourceObject(incomingPointer<LavSimulation>(simulation), incomingPointer<LavEnvironmentBase>(environment));
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
