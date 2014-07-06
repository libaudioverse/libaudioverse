/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_objects.hpp"
#include "private_sourcemanager.hpp"
#include <memory>

class LavSourceObject: public LavObject {
	public:
	LavSourceObject(std::shared_ptr<LavDevice> device, std::shared_ptr<LavSourceManager> world, std::shared_ptr<LavObject> sourceNode);
	void update(LavEnvironment env);
	virtual void willProcessParents();
	private:
	std::shared_ptr<LavObject> source_object, panner_object, attenuator_object;
	std::shared_ptr<LavSourceManager> manager;
};
