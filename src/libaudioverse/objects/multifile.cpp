/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Reads an entire file into memory and plays it with support for dopler and seeking.*/
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_file.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_creators.hpp>
#include <limits>
#include <memory>
#include <math.h>

class LavMultifileObject: public LavSubgraphObject {
	public:
	LavMultifileObject(std::shared_ptr<LavSimulation> simulation, int channels, int maxSimultaneousFiles);
	~LavMultifileObject();
	void play(std::string file);
	std::shared_ptr<LavObject> mixer = nullptr;
	int channels, max_simultaneous_files;
	std::vector<std::shared_ptr<LavObject>> file_nodes;
};

LavMultifileObject::LavMultifileObject(std::shared_ptr<LavSimulation> simulation, int channels, int maxSimultaneousFiles): LavSubgraphObject(Lav_OBJTYPE_MULTIFILE, simulation) {
	this->channels = channels;
	this->max_simultaneous_files = maxSimultaneousFiles;
	this->mixer =createMixerObject(simulation, maxSimultaneousFiles, channels);
	configureSubgraph(nullptr, mixer);
	this->file_nodes.resize(maxSimultaneousFiles);
	for(int i = 0; i < file_nodes.size(); i++) file_nodes[i] = nullptr;
}

LavMultifileObject::~LavMultifileObject() {
}

std::shared_ptr<LavObject> createMultifileObject(std::shared_ptr<LavSimulation> simulation, int channels, int maxSimultaneousFiles) {
	return std::make_shared<LavMultifileObject>(simulation, channels, maxSimultaneousFiles);
}

void LavMultifileObject::play(std::string file) {
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
	auto obj = createFileObject(simulation, file.c_str());
	for(int i = 0; i < channels; i++) {
		if(i >= obj->getInputCount()) mixer->setInput(empty_slot*channels+i, nullptr, 0);
		else mixer->setInput(empty_slot*channels+i, obj, i);
	}
	//we need to hook up a clearing event.  We do this here.
	std::weak_ptr<LavMultifileObject> weakref = std::static_pointer_cast<LavMultifileObject>(this->shared_from_this());
	auto &ev =getEvent(Lav_FILE_END_EVENT);
	ev.setHandler([=](LavObject* obj, void* userdata) {
		auto strongref = weakref.lock();
		if(strongref == nullptr) return; //no more strong reference for us to work with.
		LOCK(*strongref); //we lock it so we can safely manipulate our mixer.
		for(int i = 0; i < channels; i++) {
			strongref->mixer->setInput(empty_slot*channels+i, nullptr, 0); //must go through strongref. We don't want to actually capture the mixer.
		}
		strongref->file_nodes[empty_slot] = nullptr; //this slot has again become available.
	});
	file_nodes[empty_slot] = obj;
}
