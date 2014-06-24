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
LavWorld::LavWorld(LavDevice* device, LavHrtfData* hrtf): LavSourceManager(device, device->getChannels()) {
	this->hrtf = hrtf;
	mixer = createMixerObject(device, 512, device->getChannels());
	max_sources = 512;
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
g	lm::vec3(0.0f, 0.0f, -1.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
}

void LavWorld::willProcessParents() {
	for(auto i: sources) {
		i->update(environment);
	}
}

LavObject* LavWorld::createPannerObject() {
	return createHrtfObject(device, hrtf);
}
