/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../private/multichannel_filter_bank.hpp"
#include "../implementations/nested_allpass_network.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;

class NestedAllpassNetworkNode: public Node {
	public:
	NestedAllpassNetworkNode(std::shared_ptr<Simulation> sim, int channels);
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

std::shared_ptr<Node> createNestedAllpassNetworkNode(std::shared_ptr<Simulation> simulation, int channels);
}