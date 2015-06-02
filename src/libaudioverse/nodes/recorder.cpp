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
#include <libaudioverse/implementations/sin_osc.hpp>
#include <powercores/threadsafe_queue.hpp>
#include <limits>

namespace libaudioverse_implementation {

class RecorderNode: public Node {
	public:
	RecorderNode(std::shared_ptr<Simulation> simulation);
	virtual void process();
	powercores::threadsafe_queu<float*> available_buffers;
	powercores::threadsafe_queue<RecordingCommand> commands;
};

RecorderNode::RecorderNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_SINE_NODE, simulation, channels, channels) {
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createRecorderNode(std::shared_ptr<Simulation> simulation, int channels) {
	std::shared_ptr<RecorderNode> retval = std::shared_ptr<RecorderNode>(new RecorderNode(simulation, channels), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void RecorderNode::process() {
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createRecorderNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createRecorderNode(simulation, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}