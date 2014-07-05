/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_sourcemanager.hpp"
#include <vector>
#include <memory>

class LavSourceObject;
class LavHrtfData;

class LavWorldObject: public LavSourceManager {
	public:
	LavWorldObject(std::shared_ptr<LavDevice> device, std::shared_ptr<LavHrtfData> hrtf);
	void associateSource(std::shared_ptr<LavSourceObject> source);
	//call update on all sources.
	virtual void willProcessParents();
	std::shared_ptr<LavObject> createPannerObject();
	private:
	std::vector<std::shared_ptr<LavSourceObject>> sources;
	std::shared_ptr<LavObject> mixer = nullptr, limiter = nullptr;
	std::shared_ptr<LavHrtfData > hrtf;
	LavEnvironment environment;
};
