/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/3d/private_sources.hpp>
#include <libaudioverse/3d/private_simpleenvironment.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_creators.hpp>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_hrtf.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <stdlib.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <algorithm>
#include <vector>

LavSimpleEnvironment::LavSimpleEnvironment(std::shared_ptr<LavSimulation> simulation, std::shared_ptr<LavHrtfData> hrtf): LavEnvironmentBase(Lav_OBJTYPE_SIMPLE_ENVIRONMENT, simulation)  {
	this->hrtf = hrtf;
	mixer = createMixerObject(simulation, 1, 8);
	limiter = createHardLimiterObject(simulation, 8);
	for(int i = 0; i < 8; i++) {
		limiter->setInput(i, mixer, i);
	}

	environment.world_to_listener_transform = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	configureSubgraph(nullptr, limiter);
}

std::shared_ptr<LavSimpleEnvironment> createWorldObject(std::shared_ptr<LavSimulation> simulation, std::shared_ptr<LavHrtfData> hrtf) {
	return std::make_shared<LavSimpleEnvironment>(simulation, hrtf);
}

void LavSimpleEnvironment::willProcessParents() {
	//update the matrix.
	const float* pos = getProperty(Lav_3D_POSITION).getFloat3Value();
	const float* atup = getProperty(Lav_3D_ORIENTATION).getFloat6Value();
	environment.world_to_listener_transform = glm::lookAt(
		glm::vec3(pos[0], pos[1], pos[2]),
		//this is a point in 3d space, not a unit vector indicating direction.
		glm::vec3(pos[0]+atup[0], pos[1]+atup[1], pos[2]+atup[2]),
		glm::vec3(atup[3], atup[4], atup[5]));

	//todo: the following needs to clean up dead sources.
	//give the new environment to the sources.
	for(auto i: sources) {
		auto tmp = i.lock();
		if(tmp == nullptr) continue;
		tmp->update(environment);
	}
}

std::shared_ptr<LavObject> LavSimpleEnvironment::createPannerObject() {
	auto pan = createMultipannerObject(simulation, hrtf);
	unsigned int slot = panners.size();
	panners.push_back(pan);
	//expand the mixer by one parent.
	mixer->getProperty(Lav_MIXER_MAX_PARENTS).setIntValue(panners.size());
	for(unsigned int i = 0; i < 8; i++) {
		mixer->setInput(slot*8+i, pan, i);
	}
	return pan;
}

void LavSimpleEnvironment::registerSourceForUpdates(std::shared_ptr<LavSourceObject> source) {
	sources.insert(source);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSimpleEnvironmentObject(LavSimulation* simulation, const char*hrtfPath, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto hrtf = std::make_shared<LavHrtfData>();
	if(std::string(hrtfPath) != "default") {
		hrtf->loadFromFile(hrtfPath, simulation->getSr());
	} else {
		hrtf->loadFromDefault(simulation->getSr());
	}
	auto retval = createWorldObject(incomingPointer<LavSimulation>(simulation), hrtf);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
