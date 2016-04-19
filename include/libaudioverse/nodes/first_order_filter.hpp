/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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