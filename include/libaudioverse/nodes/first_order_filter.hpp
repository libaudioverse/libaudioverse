/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../implementations/first_order_filter.hpp"
#include "../private/multichannel_filter_bank.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;

class FirstOrderFilterNode: public Node {
	public:
	FirstOrderFilterNode(std::shared_ptr<Simulation> sim, int channels);
	void process() override;
	void configureLowpass(float freq);
	void configureHighpass(float freq);
	void configureAllpass(float freq);
	void recomputePoleAndZero();
	MultichannelFilterBank<FirstOrderFilter> bank;
};

std::shared_ptr<Node> createFirstOrderFilterNode(std::shared_ptr<Simulation> simulation, int channels);
}