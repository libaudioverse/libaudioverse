/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../private/multichannel_filter_bank.hpp"
#include "../implementations/one_pole_filter.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;

class OnePoleFilterNode: public Node {
	public:
	OnePoleFilterNode(std::shared_ptr<Simulation> sim, int channels);
	void process() override;
	void reconfigureFilters();
	MultichannelFilterBank<OnePoleFilter> bank;
};

std::shared_ptr<Node> createOnePoleFilterNode(std::shared_ptr<Simulation> simulation, int channels);
}