/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_sources.hpp>
#include <libaudioverse/private_world.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_creators.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <stdlib.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <algorithm>
#include <vector>

LavWorld::LavWorld(LavDevice* device, LavHrtfData* hrtf): LavSourceManager(device, device->getChannels()) {
	this->hrtf = hrtf;
	mixer = createMixerObject(device, 1, device->getChannels());
	limiter = createHardLimiterObject(device, device->getChannels());
	for(int i = 0; i < device->getChannels(); i++) {
		limiter->setParent(i, mixer, i);
		setParent(i, limiter, i);
	}
	float defaultPos[] = {0.0f, 0.0f, 0.0f};
	float defaultOrient[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
	properties[Lav_3D_POSITION] = createFloat3Property("position", defaultPos);
	properties[Lav_3D_ORIENTATION] = createFloat6Property("orientation", defaultOrient);
	environment.world_to_listener_transform = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
}

LavWorld* createWorld(LavDevice* device, LavHrtfData* hrtf) {
	return new LavWorld(device, hrtf);
}

void LavWorld::willProcessParents() {
	//update the matrix.
	const float* pos = properties[Lav_3D_POSITION]->getFloat3Value();
	const float* atup = properties[Lav_3D_ORIENTATION]->getFloat6Value();
	environment.world_to_listener_transform = glm::lookAt(
		glm::vec3(pos[0], pos[1], pos[2]),
		glm::vec3(atup[0], atup[1], atup[2]),
		glm::vec3(atup[3], atup[4], atup[5]));

	//give the new environment to the sources.
	for(auto i: sources) {
		i->update(environment);
	}
}

LavObject* LavWorld::createPannerObject() {
	return createHrtfObject(device, hrtf);
}

void LavWorld::associateSource(LavSource* source) {
	//if this already exists, bail out.
	int found = std::count(sources.begin(), sources.end(), source);
	if(found) return; //it's not an error.
	sources.push_back(source);
	//tell the mixer that we would like it to have more parents.
	mixer->getProperty(Lav_MIXER_MAX_PARENTS)->setIntValue(sources.size());
	unsigned int ind = sources.size()-1;
	unsigned int startInput = ind*device->getChannels();
	for(int i = 0; i < device->getChannels(); i++) {
		mixer->setParent(startInput+i, source, i);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createWorld(LavDevice* device, LavHrtfData *hrtf, LavObject** destination) {
	PUB_BEGIN
	LavObject* retval = createWorld(device, hrtf);
	*destination = retval;
	PUB_END
}
