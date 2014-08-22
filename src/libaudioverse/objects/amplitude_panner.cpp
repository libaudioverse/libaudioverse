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
#include <libaudioverse/private_kernels.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <utility>
#include <vector>

class LavAmplitudePannerObject: public LavObject {
	public:
	LavAmplitudePannerObject(std::shared_ptr<LavSimulation> device);
	~LavAmplitudePannerObject();
	virtual void process();
	private:
	void recomputeChannelMap();
	float* channel_angles = nullptr;
	int* channel_indices = nullptr;
};

LavAmplitudePannerObject::LavAmplitudePannerObject(std::shared_ptr<LavSimulation> device): LavObject(Lav_OBJTYPE_AMPLITUDE_PANNER, simulation, 1, 0) {
	getProperty(Lav_PANNER_CHANNEL_MAP).setPostChangedCallback([this](){recomputeChannelMap();});
	recomputeChannelMap();
}

LavAmplitudePannerObject::~LavAmplitudePannerObject() {
	if(channel_angles) delete[] channel_angles;
	if(channel_indices) delete[] channel_indices;
}

void LavAmplitudePannerObject::recomputeChannelMap() {
	auto &prop = getProperty(Lav_PANNER_CHANNEL_MAP);
	int l = (int)prop.getFloatArrayLength();
	std::vector<std::tuple<float, int>> map;
	for(int i = 0; i < l; i++) {
		map.emplace_back(prop.readFloatArray(i), i);
	}
	//sort it.
	auto cmp = [](std::tuple<float, int> a, std::tuple<float, int> b) {
		return std::get<0>(a) < std::get<0>(b);
	};
	std::sort(map.begin(), map.end(), cmp);
	//copy these off.
	if(channel_angles != nullptr) delete[] channel_angles;
	if(channel_indices != nullptr) delete[] channel_indices;
	channel_angles = new float[l]();
	channel_indices = new int[l]();
	for(int i = 0; i < l; i++) {
		channel_angles[i] = std::get<0>(map[i]);
		channel_indices[i] = std::get<1>(map[i]);
	}
	resize(1, l);
}

std::shared_ptr<LavObject>createAmplitudePannerObject(std::shared_ptr<LavSimulation> simulation) {
	auto retval = std::make_shared<LavAmplitudePannerObject>(simulation);
	simulation->associateObject(retval);
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

Lav_PUBLIC_FUNCTION LavError Lav_createAmplitudePannerObject(LavSimulation* simulation, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createAmplitudePannerObject(incomingPointer<LavSimulation>(simulation));
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
