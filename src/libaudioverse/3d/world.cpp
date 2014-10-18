/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_sources.hpp>
#include <libaudioverse/private_world.hpp>
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

LavWorldObject::LavWorldObject(std::shared_ptr<LavSimulation> simulation, unsigned int forChannels, std::shared_ptr<LavHrtfData> hrtf): LavSourceManager(Lav_OBJTYPE_WORLD, simulation)  {
	this->forChannels = forChannels;
	if(forChannels == 2) {
		this->hrtf = hrtf;
	}
	mixer = createMixerObject(simulation, 1, forChannels);
	limiter = createHardLimiterObject(simulation, forChannels);
	for(int i = 0; i < forChannels; i++) {
		limiter->setInput(i, mixer, i);
	}

	environment.world_to_listener_transform = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	configureSubgraph(nullptr, limiter);
}

std::shared_ptr<LavWorldObject> createWorldObject(std::shared_ptr<LavSimulation> simulation, unsigned int forChannels, std::shared_ptr<LavHrtfData> hrtf) {
	return std::make_shared<LavWorldObject>(simulation, forChannels, hrtf);
}

void LavWorldObject::willProcessParents() {
	//update the matrix.
	const float* pos = getProperty(Lav_3D_POSITION).getFloat3Value();
	const float* atup = getProperty(Lav_3D_ORIENTATION).getFloat6Value();
	environment.world_to_listener_transform = glm::lookAt(
		glm::vec3(pos[0], pos[1], pos[2]),
		glm::vec3(atup[0], atup[1], atup[2]),
		glm::vec3(atup[3], atup[4], atup[5]));

	//todo: the following needs to clean up dead sources.
	//give the new environment to the sources.
	for(auto i: sources) {
		auto tmp = i.lock();
		if(tmp == nullptr) continue;
		tmp->update(environment);
	}
}

std::shared_ptr<LavObject> LavWorldObject::createPannerObject() {
	auto pan = createHrtfObject(simulation, hrtf);
	unsigned int slot = panners.size();
	panners.push_back(pan);
	//todo: assumes stereo implicitly, this is bad.
	//expand the mixer by one parent.
	mixer->getProperty(Lav_MIXER_MAX_PARENTS).setIntValue(panners.size());
	mixer->setInput(slot*2, pan, 0);
	mixer->setInput(slot*2+1, pan, 1);
	return pan;
}

void LavWorldObject::registerSourceForUpdates(std::shared_ptr<LavSourceObject> source) {
	sources.insert(source);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createWorldObject(LavSimulation* simulation, const char*hrtfPath, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto hrtf = std::make_shared<LavHrtfData>();
	hrtf->loadFromFile(hrtfPath, simulation->getSr());
	auto retval = createWorldObject(incomingPointer<LavSimulation>(simulation), 2, hrtf);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
