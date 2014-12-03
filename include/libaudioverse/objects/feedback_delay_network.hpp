/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private_objects.hpp"
#include "../implementations/feedback_delay_network.hpp"
#include <memory>

class LavFeedbackDelayNetworkObject: public LavObject {
	public:
	LavFeedbackDelayNetworkObject(std::shared_ptr<LavSimulation> simulation, float maxDelay, int lines);
	~LavFeedbackDelayNetworkObject();
	void process();
	void setFeedbackMatrix(int length, float* values);
	void setOutputGains(int count, float* values);
	void setDelays(int length, float* values);
	void setFeedbackDelayMatrix(int length, float* values);
	private:
	LavFeedbackDelayNetwork*network = nullptr;
	float max_delay = 0.0f;
	int line_count = 0;
	float*lastOutput = nullptr, *nextInput = nullptr;
};
