/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;
class CrossfadingDelayLine;

class CrossfadingDelayNode: public Node {
	public:
	CrossfadingDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, int channels);
	~CrossfadingDelayNode();
	void process();
	protected:
	void delayChanged();
	void recomputeDelta();
	//These do not touch the line. They keep properties in sync.
	//updateDelayInSamples is called when delay changes; updateDelay when delay_samples changes.
	void updateDelaySamples();
	void updateDelay();
	//Flag to prevent "ping-pong" in the above callbacks.
	bool is_syncing_properties = false;
	//Either 0, Lav_DELAY_DELAY, or Lav_DELAY_DELAY_SAMPLES.
	int last_updated_delay_property = 0;
	unsigned int delay_line_length = 0;
	CrossfadingDelayLine **lines;
	int channels;
};

std::shared_ptr<Node> createCrossfadingDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int channels);
}