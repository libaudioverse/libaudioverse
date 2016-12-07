/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../private/node.hpp"
#include "../implementations/first_order_filter.hpp"
#include "../private/multichannel_filter_bank.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Server;

class FirstOrderFilterNode: public Node {
	public:
	FirstOrderFilterNode(std::shared_ptr<Server> sim, int channels);
	void process() override;
	void configureLowpass(float freq);
	void configureHighpass(float freq);
	void configureAllpass(float freq);
	void recomputePoleAndZero();
	MultichannelFilterBank<FirstOrderFilter> bank;
};

std::shared_ptr<Node> createFirstOrderFilterNode(std::shared_ptr<Server> server, int channels);
}