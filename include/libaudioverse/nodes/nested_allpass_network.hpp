/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../private/node.hpp"
#include "../private/multichannel_filter_bank.hpp"
#include "../implementations/nested_allpass_network.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Server;

class NestedAllpassNetworkNode: public Node {
	public:
	NestedAllpassNetworkNode(std::shared_ptr<Server> sim, int channels);
	void process() override;
	void reset() override;
	void beginNesting(int delay, float coefficient);
	void endNesting();
	void appendAllpass(int delay, float coefficient);
	void appendOnePole(float frequency, bool isHighpass = false);
	void appendBiquad(int type, double frequency, double dbGain, double q);
	void appendReader(float mul);
	void compile();
	MultichannelFilterBank<NestedAllpassNetwork> bank;
};

std::shared_ptr<Node> createNestedAllpassNetworkNode(std::shared_ptr<Server> server, int channels);
}