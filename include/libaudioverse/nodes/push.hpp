/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../libaudioverse.h"
#include "../private/node.hpp"
#include "../private/callback.hpp"
#include <speex_resampler_cpp.hpp>
#include <memory>

namespace libaudioverse_implementation {

class Server;

class PushNode: public Node {
	public:
	PushNode(std::shared_ptr<Server> sim, unsigned int inputSr, unsigned int channels);
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

std::shared_ptr<Node> createPushNode(std::shared_ptr<Server> server, unsigned int inputSr, unsigned int channels);
}