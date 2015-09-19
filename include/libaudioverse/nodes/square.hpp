/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class SquareNode: public Node {
	public:
	SquareNode(std::shared_ptr<Simulation> simulation);
	void recompute();
	virtual void process();
	float phase = 0.0f, phase_increment=0.0f, on_for = 0.0f, prev_integral=0.0f;
};

std::shared_ptr<Node> createSquareNode(std::shared_ptr<Simulation> simulation);
}