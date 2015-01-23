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

class LavMultifileNode: public LavSubgraphNode {
	public:
	LavMultifileNode(std::shared_ptr<LavSimulation> simulation, int channels, int maxSimultaneousFiles);
	~LavMultifileNode();
	void play(std::string file);
	void stopAll();
	std::shared_ptr<LavNode> mixer = nullptr;
	int channels, max_simultaneous_files;
	std::vector<std::shared_ptr<LavNode>> file_nodes;
};

LavMultifileNode::LavMultifileNode(std::shared_ptr<LavSimulation> simulation, int channels, int maxSimultaneousFiles): LavSubgraphNode(Lav_NODETYPE_MULTIFILE, simulation) {
	this->channels = channels;
	this->max_simultaneous_files = maxSimultaneousFiles;
	this->mixer =createMixerNode(simulation, maxSimultaneousFiles, channels);
	setOutputNode(mixer);
	this->file_nodes.resize(maxSimultaneousFiles);
	for(int i = 0; i < file_nodes.size(); i++) file_nodes[i] = nullptr;
}

LavMultifileNode::~LavMultifileNode() {
}

std::shared_ptr<LavNode> createMultifileNode(std::shared_ptr<LavSimulation> simulation, int channels, int maxSimultaneousFiles) {
	auto retval =std::shared_ptr<LavMultifileNode>(new LavMultifileNode(simulation, channels, maxSimultaneousFiles), LavNodeDeleter);
	simulation->associateNode(retval);
	return retval;
}

void LavMultifileNode::play(std::string file) {
	//first, find out if we have an empty slot.  If not, then stop.
	int empty_slot= 0;
	bool found_empty_slot = false;
	for(int i = 0; i < file_nodes.size(); i++) {
		if(file_nodes[i] == nullptr) {
			found_empty_slot = true;
			empty_slot= i;
			break;
		}
	}
	if(found_empty_slot== false) return; //we're beyond the limit.
	//make a file node, put it in the slot.
	auto node = createFileNode(simulation, file.c_str());
	for(int i = 0; i < channels; i++) {
		if(i >= node->getOutputCount()) mixer->setInput(empty_slot*channels+i, nullptr, 0);
		else mixer->setInput(empty_slot*channels+i, node, i);
	}
	//we need to hook up a clearing event.  We do this here.
	std::weak_ptr<LavMultifileNode> weakref = std::static_pointer_cast<LavMultifileNode>(this->shared_from_this());
	auto &ev = node->getEvent(Lav_FILE_END_EVENT);
	ev.setHandler([=](LavNode* node, void* userdata) {
		auto strongref = weakref.lock();
		if(strongref == nullptr) return; //no more strong reference for us to work with.
		LOCK(*strongref); //we lock it so we can safely manipulate our mixer.
		for(int i = 0; i < channels; i++) {
			strongref->mixer->setInput(empty_slot*channels+i, nullptr, 0); //must go through strongref. We don't want to actually capture the mixer.
		}
		strongref->file_nodes[empty_slot] = nullptr; //this slot has again become available.
	});
	file_nodes[empty_slot] = node;
}

void LavMultifileNode::stopAll() {
	for(int i = 0; i < file_nodes.size(); i++) file_nodes[i]=nullptr;
	for(int i = 0; i < mixer->getInputCount(); i++) mixer->setInput(i, nullptr, 0);
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createMultifileNode(LavSimulation* sim, int channels, int maxSimultaneousFiles, LavNode** destination) {
	PUB_BEGIN
	LOCK(*sim);
	*destination = outgoingPointer(createMultifileNode(incomingPointer<LavSimulation>(sim), channels, maxSimultaneousFiles));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_multifileNodePlay(LavNode* node, char* path) {
	PUB_BEGIN
	LOCK(*node);
	if(node->getType() != Lav_NODETYPE_MULTIFILE) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	((LavMultifileNode*)node)->play(path);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_multifileNodeStopAll(LavNode* obj) {
	PUB_BEGIN
	LOCK(*obj);
	if(obj->getType() != Lav_NODETYPE_MULTIFILE) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	((LavMultifileNode*)obj)->stopAll();
	PUB_END
}