/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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