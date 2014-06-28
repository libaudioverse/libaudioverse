/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_macros.hpp>
#include <limits>

class LavMixerObject: public LavObject {
	public:
	LavMixerObject(LavDevice* device, unsigned int maxParents, unsigned int inputsPerParent);
	virtual void process();
	protected:
	void maxParentsChanged();
	unsigned int inputs_per_parent;
};

LavMixerObject::LavMixerObject(LavDevice* device, unsigned int maxParents, unsigned int inputsPerParent): LavObject(device, inputsPerParent*maxParents, inputsPerParent) {
	type = Lav_OBJTYPE_MIXER;
	inputs_per_parent = inputsPerParent;
	properties[Lav_MIXER_INPUTS_PER_PARENT] = createIntProperty("inputs_per_parent", inputs_per_parent, inputs_per_parent, inputs_per_parent); //in effect, readonly.  Todo: implement readonly properties.
	properties[Lav_MIXER_MAX_PARENTS] = createIntProperty("max_parents", maxParents, 0, std::numeric_limits<int>::max()); //yes, really, as many as you want.
	properties[Lav_MIXER_MAX_PARENTS]->setPostChangedCallback([this] () {maxParentsChanged();});
}

LavObject* createMixerObject(LavDevice* device, unsigned int maxParents, unsigned int inputsPerParent) {
	auto retval = new LavMixerObject(device, maxParents, inputsPerParent);
	return retval;
}

void LavMixerObject::maxParentsChanged() {
	unsigned int newInputsCount = (unsigned int)(properties[Lav_MIXER_MAX_PARENTS]->getIntValue());
	newInputsCount *= inputs_per_parent;
	resize(newInputsCount, inputs_per_parent);
}

void LavMixerObject::process() {
	for(unsigned int i = 0; i < block_size; i++) {
		for(unsigned int j = 0; j < num_outputs; j++) outputs[j][i] = 0.0f;
		for(unsigned int j = 0; j < inputs.size(); j++) {
			outputs[j%inputs_per_parent][i] += inputs[j][i];
		}
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createMixerObject(LavDevice* device, unsigned int maxParents, unsigned int inputsPerParent, LavObject** destination) {
	PUB_BEGIN
	LOCK(*device);
	LavObject* retval = createMixerObject(device, maxParents, inputsPerParent);
	*destination = retval;
	PUB_END
}
