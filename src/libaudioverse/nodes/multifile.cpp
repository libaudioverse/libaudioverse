/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Reads an entire file into memory and plays it with support for dopler and seeking.*/
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/creators.hpp>
#include <limits>
#include <memory>
#include <math.h>

class MultifileNode: public SubgraphNode {
	public:
	MultifileNode(std::shared_ptr<Simulation> simulation, int channels, int maxSimultaneousFiles);
	~MultifileNode();
	void play(std::string file);
	void stopAll();
	std::shared_ptr<Node> gain = nullptr;
	int channels, max_simultaneous_files;
	std::vector<std::shared_ptr<Node>> file_nodes;
};

MultifileNode::MultifileNode(std::shared_ptr<Simulation> simulation, int channels, int maxSimultaneousFiles): SubgraphNode(Lav_OBJTYPE_MULTIFILE_NODE, simulation) {
	this->channels = channels;
	this->max_simultaneous_files = maxSimultaneousFiles;
	this->gain=createGainNode(simulation);
	gain->resize(channels, channels);
	gain->appendInputConnection(0, channels);
	gain->appendOutputConnection(0, channels);
	setOutputNode(gain);
	this->file_nodes.resize(maxSimultaneousFiles);
	for(unsigned int i = 0; i < file_nodes.size(); i++) file_nodes[i] = nullptr;
}

MultifileNode::~MultifileNode() {
}

std::shared_ptr<Node> createMultifileNode(std::shared_ptr<Simulation> simulation, int channels, int maxSimultaneousFiles) {
	auto retval =std::shared_ptr<MultifileNode>(new MultifileNode(simulation, channels, maxSimultaneousFiles), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

void MultifileNode::play(std::string file) {
	//first, find out if we have an empty slot.  If not, then stop.
	int empty_slot= 0;
	bool found_empty_slot = false;
	for(unsigned int i = 0; i < file_nodes.size(); i++) {
		if(file_nodes[i] == nullptr) {
			found_empty_slot = true;
			empty_slot= i;
			break;
		}
	}
	if(found_empty_slot== false) return; //we're beyond the limit.
	//make a file node, put it in the slot.
	auto node = createFileNode(simulation, file.c_str());
	node->connect(0, gain, 0);
	//we need to hook up a clearing event.  We do this here.
	std::weak_ptr<MultifileNode> weakref = std::static_pointer_cast<MultifileNode>(this->shared_from_this());
	auto &ev = node->getEvent(Lav_FILE_END_EVENT);
	ev.setHandler([=](Node* node, void* userdata) {
		auto strongref = weakref.lock();
		if(strongref == nullptr) return; //no more strong reference for us to work with.
		LOCK(*strongref);
		strongref->file_nodes[empty_slot]->disconnect(0); //unhook it.
		strongref->file_nodes[empty_slot] = nullptr; //this slot has again become available.
	});
	file_nodes[empty_slot] = node;
}

void MultifileNode::stopAll() {
	for(unsigned int i = 0; i < file_nodes.size(); i++) {
		file_nodes[i]->disconnect(0);
		file_nodes[i]=nullptr;
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createMultifileNode(LavHandle simulationHandle, int channels, int maxSimultaneousFiles, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = outgoingObject(createMultifileNode(simulation, channels, maxSimultaneousFiles));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_multifileNodePlay(LavHandle nodeHandle, char* path) {
	PUB_BEGIN
	auto node= incomingObject<Node>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_MULTIFILE_NODE) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	std::static_pointer_cast<MultifileNode>(node)->play(path);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_multifileNodeStopAll(LavHandle nodeHandle) {
	PUB_BEGIN
	auto node=incomingObject<Node>(nodeHandle);
	LOCK(*node);
	if(node->getType() != Lav_OBJTYPE_MULTIFILE_NODE) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	std::static_pointer_cast<MultifileNode>(node)->stopAll();
	PUB_END
}