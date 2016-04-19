/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../private/node.hpp"
#include <memory>

namespace libaudioverse_implementation {
class Simulation;
class InterpolatedDelayLine;
class OnePoleFilter;
class InterpolatedRandomGenerator;

class FdnReverbNode: public Node {
	public:
	FdnReverbNode(std::shared_ptr<Simulation> sim);
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

std::shared_ptr<Node> createFdnReverbNode(std::shared_ptr<Simulation> simulation);
}