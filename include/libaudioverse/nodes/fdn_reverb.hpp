/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../private/node.hpp"
#include <memory>

namespace libaudioverse_implementation {
class Server;
class InterpolatedDelayLine;
class OnePoleFilter;
class InterpolatedRandomGenerator;

class FdnReverbNode: public Node {
	public:
	FdnReverbNode(std::shared_ptr<Server> sim);
	~FdnReverbNode();
	void process();
	void modulateLines();
	void reconfigureModel();
	float feedback_gains[8];
	OnePoleFilter** lowpass_filters = nullptr;
	InterpolatedDelayLine** delay_lines = nullptr;
	InterpolatedRandomGenerator **delay_line_modulators;
	//We keep a record of these for debugging and other purposes.
	//These are the delays based off the current density.
	float current_delays[8];
	//Modulation state.
	bool needs_modulation = false;
	float modulation_depth = 0.0f; //equal to the property times the modulation duration from above.
};

std::shared_ptr<Node> createFdnReverbNode(std::shared_ptr<Server> server);
}