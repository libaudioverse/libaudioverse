/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_sources.hpp>
#include <libaudioverse/private_world.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_creators.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <libaudioverse/private_errors.hpp>
#include <stdlib.h>

LavSource::LavSource(LavDevice* device, LavWorld* world, LavObject* sourceNode): LavPassthroughObject(device, device->getChannels()) {
	if(sourceNode->getOutputCount() > 1) throw LavErrorException(Lav_ERROR_SHAPE);
	attenuator_object = createAttenuatorObject(device, 1);
	panner_object = world->createPannerObject();
	attenuator_object->setParent(0, source_object, 0);
	panner_object->setParent(0, attenuator_object, 0);
	for(unsigned int i = 0; i <num_inputs; i++) {
		setParent(i, panner_object, i);
	}
}

void LavSource::update(LavListenerInfo listener) {
	listener_info = listener;
}
