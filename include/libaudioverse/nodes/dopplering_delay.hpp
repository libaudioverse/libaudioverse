/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;
class DoppleringDelayLine;

class DoppleringDelayNode: public Node {
	public:
	DoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, int channels);
	~DoppleringDelayNode();
	void process();
	protected:
	void delayChanged();
	void recomputeDelta();
	//Callbacks for when delay_samples and delay get changed.
	void updateDelay();
	void updateDelaySamples();
	//This flag prevents the above two callbacks from ping-ponging.
	bool is_syncing_properties = false;
	//Set to either Lav_DELAY_DELAY or Lav_DELAY_DELAY_SAMPLES if the property changed. Otherwise 0.
	int last_updated_delay_property = 0;
	//Standard stuff for delay lines.
	unsigned int delay_line_length = 0;
	DoppleringDelayLine **lines;
	int channels;
};

std::shared_ptr<Node> createDoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, int channels);
}