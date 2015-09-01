/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../implementations/feedback_delay_network.hpp"
#include "../implementations/delayline.hpp"
#include <memory>

namespace libaudioverse_implementation {

class OnePoleFilter;

//This node can be controlled both through properties and the methods herein.
//Internal code should use the methods, which do not keep the properties updated but avoid lots of overhead.
class FeedbackDelayNetworkNode: public Node {
	public:
	FeedbackDelayNetworkNode(std::shared_ptr<Simulation> simulation, float maxDelay, int lines);
	~FeedbackDelayNetworkNode();
	void process();
	void setMatrix(float* values);
	void setOutputGains(float* values);
	void setDelays(float* values);
	void configureFilters(int* types, float* frequencies);	
	private:
	FeedbackDelayNetwork<InterpolatedDelayLine>*network = nullptr;
	float max_delay = 0.0f;
	int channels = 0;
	float*last_output = nullptr, *next_input=nullptr;
	float* gains = nullptr;
	//Filters to be inserted into the feedback path.
	OnePoleFilter** filters = nullptr;
};

}