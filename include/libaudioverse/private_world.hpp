/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_sourcemanager.hpp"
#include <vector>
#include <set>
#include <memory>

class LavSourceObject;
class LavHrtfData;

class LavWorldObject: public LavSourceManager {
	public:
	LavWorldObject(std::shared_ptr<LavSimulation> simulation, unsigned int forChannels, std::shared_ptr<LavHrtfData> hrtf);
	void registerSourceForUpdates(std::shared_ptr<LavSourceObject> source);
	//call update on all sources.
	virtual void willProcessParents();
	std::shared_ptr<LavObject> createPannerObject();
	private:
	unsigned int forChannels = 0;
	//while these may be parents (through virtue of the panners we give out), they also have to hold a reference to us-and that reference must be strong.
	//the world is more capable of handling a source that dies than a source a world that dies.
	std::set<std::weak_ptr<LavSourceObject>, std::owner_less<std::weak_ptr<LavSourceObject>>> sources;
	//these are parents of us, so this doesn't create cyclic behavior.  We give a new one out and connect it through our mixer for each source.
	std::vector<std::shared_ptr<LavObject>> panners;
	std::shared_ptr<LavObject> mixer = nullptr, limiter = nullptr;
	std::shared_ptr<LavHrtfData > hrtf;
	LavEnvironment environment;
};
