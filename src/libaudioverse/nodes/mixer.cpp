/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <limits>
#include <memory>

class LavMixerNode: public LavNode {
	public:
	LavMixerNode(std::shared_ptr<LavSimulation> simulation, unsigned int maxParents, unsigned int inputsPerParent);
	virtual void process();
	protected:
	void maxParentsChanged();
};

LavMixerNode::LavMixerNode(std::shared_ptr<LavSimulation> simulation, unsigned int maxParents, unsigned int inputsPerParent): LavNode(Lav_NODETYPE_MIXER, simulation, inputsPerParent*maxParents, inputsPerParent) {
	getProperty(Lav_MIXER_INPUTS_PER_PARENT).setIntRange(inputsPerParent, inputsPerParent);
	getProperty(Lav_MIXER_INPUTS_PER_PARENT).setIntValue(inputsPerParent);
	getProperty(Lav_MIXER_MAX_PARENTS).setIntValue(maxParents);
	getProperty(Lav_MIXER_MAX_PARENTS).setPostChangedCallback([this] () {maxParentsChanged();});
}

std::shared_ptr<LavNode> createMixerNode(std::shared_ptr<LavSimulation> simulation, unsigned int maxParents, unsigned int inputsPerParent) {
	auto retval = std::shared_ptr<LavMixerNode>(new LavMixerNode(simulation, maxParents, inputsPerParent), LavNodeDeleter);
	simulation->associateNode(retval);
	return retval;
}

void LavMixerNode::maxParentsChanged() {
	unsigned int newInputsCount = (unsigned int)(getProperty(Lav_MIXER_MAX_PARENTS).getIntValue());
	int inputsPerParent = getProperty(Lav_MIXER_INPUTS_PER_PARENT).getIntValue();
	newInputsCount *= inputsPerParent;
	resize(newInputsCount, inputsPerParent);
}

void LavMixerNode::process() {
	int inputsPerParent = getProperty(Lav_MIXER_INPUTS_PER_PARENT).getIntValue();
	int maxParents = getProperty(Lav_MIXER_MAX_PARENTS).getIntValue();
	//common case, 1 parent.
	//most commonly seen internally to this library because mixers make forwarders in subgraphs.
	if(maxParents == 1) {
		for(unsigned int i = 0; i < num_outputs; i++) {
			std::copy(inputs[i], inputs[i]+block_size, outputs[i]);
		}
		return;
	}
	//outputs are zeroed for us as a guarantee of inheriting from LavNode.
	for(unsigned int i = 0; i < num_inputs; i++) {
		//if this is the zerobuffer, we can save block_size adds, vector accesses, etc.
		if(inputs[i] == zerobuffer) continue;
			additionKernel(block_size, outputs[i%inputsPerParent], inputs[i], outputs[i%inputsPerParent]);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createMixerNode(LavSimulation* simulation, unsigned int maxParents, unsigned int inputsPerParent, LavNode** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createMixerNode(incomingPointer<LavSimulation>(simulation), maxParents, inputsPerParent);
	*destination = outgoingPointer<LavNode>(retval);
	PUB_END
}
