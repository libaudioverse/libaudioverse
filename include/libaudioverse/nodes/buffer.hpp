/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../libaudioverse.h"
#include "../private/node.hpp"
#include "../private/callback.hpp"
#include "../implementations/buffer_player.hpp"
#include <memory>

namespace libaudioverse_implementation {


class Simulation;

class BufferNode: public Node {
	public:
	BufferNode(std::shared_ptr<Simulation> simulation);
	void setBuffer(std::shared_ptr<Buffer> buff);
	void positionChanged();
	void bufferChanged();
	virtual void process();
	BufferPlayer player;
	std::shared_ptr<Callback<void()>> end_callback;
};

std::shared_ptr<Node> createBufferNode(std::shared_ptr<Simulation> simulation);

}