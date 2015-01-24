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

class LavSplitMergeNode: public LavNode {
	public:
	LavSplitMergeNode(std::shared_ptr<LavSimulation> simulation, int type);
	//these overrides let the input buffers be the output buffers, cutting the memory usage for these in half.
	int getOutputBufferCount() override;
	float** getOutputBufferArray() override;
};

LavSplitMergeNode::LavSplitMergeNode(std::shared_ptr<LavSimulation> simulation, int type): LavNode(type, simulation, 0, 1) {
}

std::shared_ptr<LavNode> createSplitMergeNode(std::shared_ptr<LavSimulation> simulation, int type) {
	auto retval = std::shared_ptr<LavSplitMergeNode>(new LavSplitMergeNode(simulation, type), LavNodeDeleter);
	return retval;
}

std::shared_ptr<LavNode> createChannelSplitterNode(std::shared_ptr<LavSimulation> simulation, int channels) {
	auto retval= createSplitMergeNode(simulation, Lav_NODETYPE_CHANNEL_SPLITTER);
	retval->resize(channels, 0);
	for(int i =0; i < channels; i++) retval->appendOutputConnection(i, 1);
	retval->appendInputConnection(0, channels);
	simulation->associateNode(retval);
	return retval;
}

std::shared_ptr<LavNode> createChannelMergerNode(std::shared_ptr<LavSimulation> simulation, int channels) {
	auto retval = createSplitMergeNode(simulation, Lav_NODETYPE_CHANNEL_MERGER);
	retval->resize(channels, 0);
	retval->appendOutputConnection(0, channels);
	for(int i = 0; i < channels; i++) retval->appendInputConnection(i, 1);
	simulation->associateNode(retval);
	return retval;
}

int LavSplitMergeNode::getOutputBufferCount() {
	return getInputBufferCount();
}

float** LavSplitMergeNode::getOutputBufferArray() {
	return getInputBufferArray();
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createChannelSplitterNode(LavSimulation* simulation, int channels, LavNode** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createChannelSplitterNode(incomingPointer<LavSimulation>(simulation), channels);
	*destination = outgoingPointer(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_createChannelMergerNode(LavSimulation* simulation, int channels, LavNode** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createChannelMergerNode(incomingPointer<LavSimulation>(simulation), channels);
	*destination = outgoingPointer(retval);
	PUB_END
}
