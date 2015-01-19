/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/functiontables.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <limits>

class LavSineNode: public LavNode {
	public:
	LavSineNode(std::shared_ptr<LavSimulation> simulation);
	virtual void process();
	float table_delta;
	unsigned int start ;
	float offset;
};

LavSineNode::LavSineNode(std::shared_ptr<LavSimulation> simulation): LavNode(Lav_NODETYPE_SINE, simulation, 0, 1) {
	table_delta = sineTableLength/simulation->getSr();
	start = 0;
	offset = 0;
	appendOutputConnection(0, 1);
}

std::shared_ptr<LavNode> createSineNode(std::shared_ptr<LavSimulation> simulation) {
	std::shared_ptr<LavSineNode> retval = std::shared_ptr<LavSineNode>(new LavSineNode(simulation), LavNodeDeleter);
	simulation->associateNode(retval);
	return retval;
}

void LavSineNode::process() {
	float freq = getProperty(Lav_SINE_FREQUENCY).getFloatValue();
	for(unsigned int i = 0; i< block_size; i++) {
		const unsigned int samp1 = start;
		const unsigned int samp2 = start+1;
		const float weight1 = offset;
		const float weight2 = 1-offset;
		output_buffers[0][i] = sineTable[samp1]*weight1+sineTable[samp2]*weight2;
		offset += table_delta*freq;
		start += (int)floorf(offset);
		start %= sineTableLength;
		offset = fmod(offset, 1.0f);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSineNode(LavSimulation* simulation, LavNode **destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createSineNode(incomingPointer<LavSimulation>(simulation));
	*destination = outgoingPointer<LavNode>(retval);
	PUB_END
}