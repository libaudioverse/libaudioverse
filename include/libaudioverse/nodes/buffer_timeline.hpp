/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../private/node.hpp"
#include <map>
#include <memory>

namespace libaudioverse_implementation {

class Server;
class Buffer;
class BufferPlayer;

class BufferTimelineNode: public Node {
	public:
	BufferTimelineNode(std::shared_ptr<Server> server, int channels);
	~BufferTimelineNode();
	void process() override;
	void scheduleBuffer(double time, float delta, std::shared_ptr<Buffer> buffer);
	void reset() override;
	private:
	std::multimap<double, BufferPlayer*> scheduled_buffers;
	std::vector<float*> workspace;
	double time = 0.0;
	int output_channels = 0;
};

std::shared_ptr<Node> createBufferTimelineNode(std::shared_ptr<Server> server, int channels);
}