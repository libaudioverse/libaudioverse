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
	LavAmplitudePannerObject(std::shared_ptr<LavDevice> device, unsigned int numChannels, float* channelAngles, int* channelIndices);
	~LavAmplitudePannerObject();
	virtual void process();
	private:
	float* channel_angles;
	int* channel_indices;
};

LavAmplitudePannerObject::LavAmplitudePannerObject(std::shared_ptr<LavDevice> device, unsigned int numChannels, float* channelAngles, int* channelIndices): LavObject(Lav_OBJTYPE_AMPLITUDE_PANNER, device, 1, numChannels) {
	channel_angles = channelAngles;
	channel_indices = channelIndices;
}

LavAmplitudePannerObject::~LavAmplitudePannerObject() {
	delete[] channel_angles;
	delete[] channel_indices;
}

std::shared_ptr<LavObject>createAmplitudePannerObject(std::shared_ptr<LavDevice> device, unsigned int numChannels, float* channelAngles, int* channelIndices) {
	auto retval = std::make_shared<LavAmplitudePannerObject>(device, numChannels, channelAngles, channelIndices);
	device->associateObject(retval);
	return retval;
}

void LavAmplitudePannerObject::process() {
	//has the effect of zeroing the inputs.
LavObject::process();
	float azimuth = getProperty(Lav_PANNER_AZIMUTH).getFloatValue();
	float elevation = getProperty(Lav_PANNER_ELEVATION).getFloatValue();
	amplitudePanKernel(azimuth, elevation, block_size, inputs[0], num_outputs, &outputs[0], channel_angles, channel_indices);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerObject(LavDevice* device, int numChannels, float* channelAngles, int* channelIndices, LavObject** destination) {
	PUB_BEGIN
	LOCK(*device);
	float* newChannelAngles = new float[numChannels];
	std::copy(channelAngles, channelAngles+numChannels, newChannelAngles);
	int* newChannelIndices = new int[numChannels];
	std::copy(channelIndices, channelIndices+numChannels, newChannelIndices);
	auto retval = createAmplitudePannerObject(incomingPointer<LavDevice>(device), numChannels, newChannelAngles, newChannelIndices);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
