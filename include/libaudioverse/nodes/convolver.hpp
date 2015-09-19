/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;
class BlockConvolver;

class ConvolverNode: public Node {
	public:
	ConvolverNode(std::shared_ptr<Simulation> simulation, int channels);
	~ConvolverNode();
	virtual void process();
	void setImpulseResponse();
	int channels;
	BlockConvolver **convolvers;
};

std::shared_ptr<Node> createConvolverNode(std::shared_ptr<Simulation> simulation, int channels);
}