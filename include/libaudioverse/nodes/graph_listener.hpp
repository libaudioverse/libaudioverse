/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include "../private/node.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;

class GraphListenerNode: public Node {
	public:
	GraphListenerNode(std::shared_ptr<Simulation> sim, unsigned int channels);
	~GraphListenerNode();
	void process();
	LavGraphListenerNodeListeningCallback callback = nullptr;
	float* outgoing_buffer = nullptr;
	void* callback_userdata = nullptr;
	unsigned int channels = 0;
};

std::shared_ptr<Node> createGraphListenerNode(std::shared_ptr<Simulation> simulation, unsigned int channels);
}