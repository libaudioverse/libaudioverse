/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../implementations/hrtf_panner.hpp"
#include <memory>

namespace libaudioverse_implementation {
class Simulation;
class HrtfData;
class FftConvolver;

class HrtfNode: public Node {
	public:
	HrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
	virtual void process() override;
	void reset();
	private:
	HrtfPanner panner;
};

std::shared_ptr<Node>createHrtfNode(std::shared_ptr<Simulation>simulation, std::shared_ptr<HrtfData> hrtf);
}