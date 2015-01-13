/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include "../private_objects.hpp"
#include <glm/glm.hpp>
#include <memory>

/**This is an abstract class and helper struct that is the minimum a source needs to associate to an object.*/

class LavSourceNode;

/**This holds info on listener positions, defaults, etc.
Anything a source needs for updating, basically.*/
struct LavEnvironment {
	glm::mat4 world_to_listener_transform;
};

class LavEnvironmentBase: public LavSubgraphNode {
	public:
	LavEnvironmentBase(int type, std::shared_ptr<LavSimulation> simulation): LavSubgraphNode(type, simulation) {}
	//Register a source for updates.  Subclasses should only hold a weak_ptr to the source and should allow it to die.
	virtual void registerSourceForUpdates(std::shared_ptr<LavSourceNode> source) = 0;
	//must return an appropriate panner object for this environment.
	virtual std::shared_ptr<LavNode> createPannerObject() = 0;
};
