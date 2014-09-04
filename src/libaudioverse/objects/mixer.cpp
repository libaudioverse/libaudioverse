/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <limits>
#include <memory>

class LavMixerObject: public LavObject {
	public:
	LavMixerObject(std::shared_ptr<LavSimulation> simulation, unsigned int maxParents, unsigned int inputsPerParent);
	virtual void process();
	protected:
	void maxParentsChanged();
};

LavMixerObject::LavMixerObject(std::shared_ptr<LavSimulation> simulation, unsigned int maxParents, unsigned int inputsPerParent): LavObject(Lav_OBJTYPE_MIXER, simulation, inputsPerParent*maxParents, inputsPerParent) {
	type = Lav_OBJTYPE_MIXER;
	getProperty(Lav_MIXER_INPUTS_PER_PARENT).setIntRange(inputsPerParent, inputsPerParent);
	getProperty(Lav_MIXER_INPUTS_PER_PARENT).setIntValue(inputsPerParent);
	getProperty(Lav_MIXER_MAX_PARENTS).setIntValue(maxParents);
	getProperty(Lav_MIXER_MAX_PARENTS).setPostChangedCallback([this] () {maxParentsChanged();});
}

std::shared_ptr<LavObject> createMixerObject(std::shared_ptr<LavSimulation> simulation, unsigned int maxParents, unsigned int inputsPerParent) {
	auto retval = std::make_shared<LavMixerObject>(simulation, maxParents, inputsPerParent);
	simulation->associateObject(retval);
	return retval;
}

void LavMixerObject::maxParentsChanged() {
	unsigned int newInputsCount = (unsigned int)(getProperty(Lav_MIXER_MAX_PARENTS).getIntValue());
	int inputsPerParent = getProperty(Lav_MIXER_INPUTS_PER_PARENT).getIntValue();
	newInputsCount *= inputsPerParent;
	resize(newInputsCount, inputsPerParent);
}

void LavMixerObject::process() {
	int inputsPerParent = getProperty(Lav_MIXER_INPUTS_PER_PARENT).getIntValue();
	for(unsigned int i = 0; i < block_size; i++) {
		for(unsigned int j = 0; j < num_outputs; j++) outputs[j][i] = 0.0f;
		for(unsigned int j = 0; j < inputs.size(); j++) {
			outputs[j%inputsPerParent][i] += inputs[j][i];
		}
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createMixerObject(LavSimulation* simulation, unsigned int maxParents, unsigned int inputsPerParent, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createMixerObject(incomingPointer<LavSimulation>(simulation), maxParents, inputsPerParent);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}
