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
#include "../implementations/buffer_player.hpp"
#include <memory>

namespace libaudioverse_implementation {


class Server;

class BufferNode: public Node {
	public:
	BufferNode(std::shared_ptr<Server> server);
	void setBuffer(std::shared_ptr<Buffer> buff);
	void positionChanged();
	void bufferChanged();
	virtual void process();
	BufferPlayer player;
	std::shared_ptr<Callback<void()>> end_callback;
};

std::shared_ptr<Node> createBufferNode(std::shared_ptr<Server> server);

}