/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/implementations/panner.hpp>
#include <libaudioverse/private_constants.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <utility>
#include <vector>

float standard_map_stereo[] = {-90.0f, 90.0f};
float standard_map_51[] = {-22.5f, 22.5f, -110.0f, 110.0f};
float standard_map_71[] = {-22.5f, 22.5f, -150.0f, 150.0f, -110.0f, 110.0f};

class LavAmplitudePannerObject: public LavObject {
	public:
	LavAmplitudePannerObject(std::shared_ptr<LavSimulation> device);
	virtual void process();
	void recomputeChannelMap();
	void configureStandardChannelMap(unsigned int channels);
	bool map_changed = true;
	LavPannerImplementation panner;
};

LavAmplitudePannerObject::LavAmplitudePannerObject(std::shared_ptr<LavSimulation> simulation): LavObject(Lav_OBJTYPE_AMPLITUDE_PANNER, simulation, 1, 0) {
	getProperty(Lav_PANNER_CHANNEL_MAP).setPostChangedCallback([this](){recomputeChannelMap();});
	getProperty(Lav_PANNER_SKIP_LFE).setPostChangedCallback([this] () {recomputeChannelMap();});
	getProperty(Lav_PANNER_SKIP_CENTER).setPostChangedCallback([this] (){recomputeChannelMap();});
	recomputeChannelMap();
}


std::shared_ptr<LavObject>createAmplitudePannerObject(std::shared_ptr<LavSimulation> simulation) {
	auto retval = std::make_shared<LavAmplitudePannerObject>(simulation);
	simulation->associateObject(retval);
	return retval;
}

void LavAmplitudePannerObject::recomputeChannelMap() {
	panner.reset();
	bool skipLfe = getProperty(Lav_PANNER_SKIP_LFE).getIntValue() == 1;
	bool skipCenter = getProperty(Lav_PANNER_SKIP_CENTER).getIntValue() == 1;
	LavProperty& channelMap = getProperty(Lav_PANNER_CHANNEL_MAP);
	unsigned int max = channelMap.getFloatArrayLength();
	if(skipLfe && channelMap.getFloatArrayLength() > 2) max++;
	if(skipCenter && channelMap.getFloatArrayLength() > 2) max++;
	resize(1, max);
	unsigned int index = 0;
	for(unsigned int i = 0; i < max; i++) {
		if(i == 2 && skipCenter) continue;
		if(i == 3 && skipLfe) continue;
		float angle = channelMap.readFloatArray(index);
		panner.addEntry(angle, i);
		index++;
	}
}

void LavAmplitudePannerObject::process() {
	float azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	panner.pan(azimuth, block_size, inputs[0], outputs.size(), &outputs[0]);
}

void LavAmplitudePannerObject::configureStandardChannelMap(unsigned int channels) {
	switch(channels) {
		case 2:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(2, standard_map_stereo);
		break;
		case 6:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(4, standard_map_51);
		break;
		case 8:
		getProperty(Lav_PANNER_CHANNEL_MAP).replaceFloatArray(6, standard_map_71);
		break;
	};
	getProperty(Lav_PANNER_SKIP_LFE).setIntValue(1);
	getProperty(Lav_PANNER_SKIP_CENTER).setIntValue(1);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerObject(LavSimulation* simulation, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createAmplitudePannerObject(incomingPointer<LavSimulation>(simulation));
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_amplitudePannerObjectConfigureStandardMap(LavObject* obj, unsigned int channels) {
	PUB_BEGIN
	if(channels != 2 && channels != 6 && channels != 8) throw LavErrorException(Lav_ERROR_RANGE);
	LOCK(*obj);
	if(obj->getType() != Lav_OBJTYPE_AMPLITUDE_PANNER) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	((LavAmplitudePannerObject*)obj)->configureStandardChannelMap(channels);
	PUB_END
}
