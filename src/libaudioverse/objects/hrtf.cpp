/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <stdlib.h>
#include <string.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_hrtf.hpp>
#include <limits>
#include <functional>

class LavHrtfObject: public LavObject {
	public:
	virtual void init(LavDevice* device, LavHrtfData* hrtf);
	virtual void process();
	private:
	float *history, *left_response, *right_response;
	LavHrtfData* hrtf;
	bool needs_hrtf_recompute;
};

void LavHrtfObject::init(LavDevice* device, LavHrtfData* hrtf) {
	LavObject::init(device, 1, 2);
	this->hrtf = hrtf;
	left_response = new float[hrtf->getLength()];
	right_response = new float[hrtf->getLength()];
	history = new float[hrtf->getLength() + device->getBlockSize()];
	hrtf->computeCoefficientsStereo(0.0f, 0.0f, left_response, right_response);
	properties[Lav_HRTF_AZIMUTH] = createFloatProperty("azimuth", 0.0f, -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	properties[Lav_HRTF_ELEVATION] = createFloatProperty("elevation", 0.0f, -90.0f, 90.0f);
	auto markRecompute = [this](){needs_hrtf_recompute = true;};
	properties[Lav_HRTF_AZIMUTH]->setPostChangedCallback(markRecompute);
	properties[Lav_HRTF_ELEVATION]->setPostChangedCallback(markRecompute);
}
