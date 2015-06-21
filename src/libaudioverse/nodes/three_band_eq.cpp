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
#include <libaudioverse/private/iir.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <limits>
#include <algorithm>


namespace libaudioverse_implementation {

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class ThreeBandEqNode: public Node {
	public:
	ThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels);
	void recompute();
	virtual void process() override;
	virtual void reset() override;
	IIRFilter** midband_peaks= nullptr;
	IIRFilter** highband_shelves = nullptr;
	int channels;
};

ThreeBandEqNode::ThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_THREE_BAND_EQ_NODE, simulation, channels, channels) {
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
}

std::shared_ptr<Node> createThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels) {
	std::shared_ptr<ThreeBandEqNode> retval = std::shared_ptr<ThreeBandEqNode>(new ThreeBandEqNode(simulation, channels), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void ThreeBandEqNode::recompute() {
}

void ThreeBandEqNode::process() {
}

void ThreeBandEqNode::reset() {
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createThreeBandEqNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createThreeBandEqNode(simulation, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}