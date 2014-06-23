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

LavWorld::LavWorld(LavDevice* device, LavHrtfData* hrtf): LavPassthroughObject(device, device->getChannels()) {
	this->hrtf = hrtf;
	mixer = createMixerObject(device, 512, device->getChannels());
	max_sources = 512;
	limiter = createHardLimiterObject(device, device->getChannels());
}

void LavWorld::willProcessParents() {
	for(auto i: sources) {
		i->update(environment);
	}
}

LavObject* LavWorld::createPannerObject() {
	return createHrtfObject(device, hrtf);
}
