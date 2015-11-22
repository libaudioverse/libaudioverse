/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include "../private/node.hpp"
#include "../private/callback.hpp"
#include <speex_resampler_cpp.hpp>
#include <memory>

namespace libaudioverse_implementation {

class Simulation;

class PushNode: public Node {
	public:
	PushNode(std::shared_ptr<Simulation> sim, unsigned int inputSr, unsigned int channels);
	~PushNode();
	void process();
	void feed(unsigned int length, float* buffer);
	std::shared_ptr<Callback<void()>> low_callback, underrun_callback;
	unsigned int input_sr = 0;
	std::shared_ptr<speex_resampler_cpp::Resampler> resampler = nullptr;
	float* workspace = nullptr;
	//the push_* variables are for the public api to feed us.
	float* push_buffer = nullptr;
	unsigned int push_channels = 0;
	unsigned int push_frames = 1024;
	unsigned int push_offset = 0;
	bool fired_underrun_callback = false;
};

std::shared_ptr<Node> createPushNode(std::shared_ptr<Simulation> simulation, unsigned int inputSr, unsigned int channels);
}