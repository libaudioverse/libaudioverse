/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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

std::shared_ptr<Node> createFeedbackDelayNetworkNode(std::shared_ptr<Simulation> simulation, float maxDelay, int channels);
}