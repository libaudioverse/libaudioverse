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
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/buffer.hpp>
#include <limits>

class LavBufferNode: public LavNode {
	public:
	LavBufferNode(std::shared_ptr<LavSimulation> simulation);
	virtual void bufferChanged();
	virtual void process();
	int frame = 0;
};

LavBufferNode::LavBufferNode(std::shared_ptr<LavSimulation> simulation): LavNode(Lav_NODETYPE_BUFFER, simulation, 0, 1) {
	appendOutputConnection(0, 1);
}

std::shared_ptr<LavNode> createBufferNode(std::shared_ptr<LavSimulation> simulation) {
	std::shared_ptr<LavBufferNode> retval = std::shared_ptr<LavBufferNode>(new LavBufferNode(simulation), LavObjectDeleter);
	simulation->associateNode(retval);
	return retval;
}

void LavBufferNode::bufferChanged() {
	//if it's null, then we stop.
	auto buff= getProperty(Lav_BUFFER_BUFFER).getBufferValue();
	if(buff==nullptr) {
		resize(0, 1);
		getOutputConnection(0)->reconfigure(0, 1);
	}
	else {
		int channels = buff->getChannels() > 0 ? buff->getChannels() : 1;
		resize(0, channels);
		getOutputConnection(0)->reconfigure(0, channels);
	}
	frame = 0;
}

void LavBufferNode::process() {
	//We are zeroed by the processing logic.
	auto buff= getProperty(Lav_BUFFER_BUFFER).getBufferValue();
	if(buff) {
		frame +=buff->writeData(frame, num_output_buffers, block_size, &output_buffers[0]);
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createBufferNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<LavSimulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createBufferNode(simulation);
	*destination = outgoingObject<LavNode>(retval);
	PUB_END
}
