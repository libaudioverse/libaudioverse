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
#include <algorithm>

namespace libaudioverse_implementation {

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class LateReflectionsNode: public Node {
	public:
	LateReflectionsNode(std::shared_ptr<Simulation> simulation);
	virtual void process();
};

LateReflectionsNode::LateReflectionsNode(std::shared_ptr<Simulation> simulation): Node(Lav_OBJTYPE_LATE_REFLECTIONS_NODE, simulation, 16, 16) {
}

std::shared_ptr<Node> createLateReflectionsNode(std::shared_ptr<Simulation> simulation) {
	std::shared_ptr<LateReflectionsNode> retval = std::shared_ptr<LateReflectionsNode>(new LateReflectionsNode(simulation), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void LateReflectionsNode::process() {
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createLateReflectionsNode(LavHandle simulationHandle, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createLateReflectionsNode(simulation);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}