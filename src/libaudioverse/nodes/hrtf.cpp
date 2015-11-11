/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/nodes/hrtf.hpp>
#include <libaudioverse/implementations/hrtf_panner.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/constants.hpp>
#include <memory>

namespace libaudioverse_implementation {

HrtfNode::HrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf): Node(Lav_OBJTYPE_HRTF_NODE, simulation, 1, 2),
panner(simulation->getBlockSize(), simulation->getSr(), hrtf) {
	appendInputConnection(0, 1);
	appendOutputConnection(0, 2);
}

std::shared_ptr<Node>createHrtfNode(std::shared_ptr<Simulation>simulation, std::shared_ptr<HrtfData> hrtf) {
	return standardNodeCreation<HrtfNode>(simulation, hrtf);
}

void HrtfNode::process() {
	panner.setAzimuth(getProperty(Lav_PANNER_AZIMUTH).getFloatValue());
	panner.setElevation(getProperty(Lav_PANNER_ELEVATION).getFloatValue());
	panner.setShouldCrossfade(getProperty(Lav_PANNER_SHOULD_CROSSFADE).getIntValue() == 1);
	panner.pan(input_buffers[0], output_buffers[0], output_buffers[1]);
}

void HrtfNode::reset() {
	panner.reset();
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createHrtfNode(LavHandle simulationHandle, const char* hrtfPath, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto hrtf = createHrtfFromString(hrtfPath, simulation->getSr());
	auto retval = createHrtfNode(simulation, hrtf);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}