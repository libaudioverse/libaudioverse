/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>


namespace libaudioverse_implementation {

class SplitMergeNode: public Node {
	public:
	SplitMergeNode(std::shared_ptr<Simulation> simulation, int type);
	//these overrides let the input buffers be the output buffers, cutting the memory usage for these in half.
	int getOutputBufferCount() override;
	float** getOutputBufferArray() override;
};

SplitMergeNode::SplitMergeNode(std::shared_ptr<Simulation> simulation, int type): Node(type, simulation, 0, 1) {
}

std::shared_ptr<Node> createSplitMergeNode(std::shared_ptr<Simulation> simulation, int type) {
	return standardNodeCreation<SplitMergeNode>(simulation, type);
}

std::shared_ptr<Node> createChannelSplitterNode(std::shared_ptr<Simulation> simulation, int channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	auto retval= createSplitMergeNode(simulation, Lav_OBJTYPE_CHANNEL_SPLITTER_NODE);
	retval->resize(channels, 0);
	for(int i =0; i < channels; i++) retval->appendOutputConnection(i, 1);
	retval->appendInputConnection(0, channels);
	simulation->associateNode(retval);
	return retval;
}

std::shared_ptr<Node> createChannelMergerNode(std::shared_ptr<Simulation> simulation, int channels) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater than 0.");
	auto retval = createSplitMergeNode(simulation, Lav_OBJTYPE_CHANNEL_MERGER_NODE);
	retval->resize(channels, 0);
	retval->appendOutputConnection(0, channels);
	for(int i = 0; i < channels; i++) retval->appendInputConnection(i, 1);
	simulation->associateNode(retval);
	return retval;
}

int SplitMergeNode::getOutputBufferCount() {
	return getInputBufferCount();
}

float** SplitMergeNode::getOutputBufferArray() {
	return getInputBufferArray();
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createChannelSplitterNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createChannelSplitterNode(simulation, channels);
	*destination = outgoingObject(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_createChannelMergerNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createChannelMergerNode(simulation, channels);
	*destination = outgoingObject(retval);
	PUB_END
}

}