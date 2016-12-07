/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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
	FeedbackDelayNetworkNode(std::shared_ptr<Server> server, float maxDelay, int lines);
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

std::shared_ptr<Node> createFeedbackDelayNetworkNode(std::shared_ptr<Server> server, float maxDelay, int channels);
}