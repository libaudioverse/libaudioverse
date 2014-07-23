/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_kernels.hpp>
#include <limits>
#include <memory>

class LavAmplitudePannerObject: public LavObject {
	public:
	LavAmplitudePannerObject(std::shared_ptr<LavDevice> device, unsigned int numChannels, float* channelMap);
	~LavAmplitudePannerObject();
	virtual void process();
	private:
	float* channel_map;
};

LavAmplitudePannerObject::LavAmplitudePannerObject(std::shared_ptr<LavDevice> device, unsigned int numChannels, float* channelMap): LavObject(Lav_OBJTYPE_AMPLITUDE_PANNER, device, 1, numChannels) {
	channel_map = channelMap;
}

LavAmplitudePannerObject::~LavAmplitudePannerObject() {
	delete[] channel_map;
}

std::shared_ptr<LavObject>createAmplitudePannerObject(std::shared_ptr<LavDevice> device, unsigned int numChannels, float* channelMap) {
	auto retval = std::make_shared<LavAmplitudePannerObject>(device, numChannels, channelMap);
	device->associateObject(retval);
	return retval;
}

void LavAmplitudePannerObject::process() {
	float azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	float elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	amplitudePanKernel(azimuth, elevation, block_size, inputs[0], num_outputs, &outputs[0], channel_map);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerObject(LavDevice* device, unsigned int numChannels, float* channelMap, LavObject** destination) {
	PUB_BEGIN
	LOCK(*device);
	float* newChannelMap = new float[numChannels];
	std::copy(channelMap, channelMap+numChannels, newChannelMap);
	auto retval = createAmplitudePannerObject(incomingPointer<LavDevice>(device), numChannels, newChannelMap);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
