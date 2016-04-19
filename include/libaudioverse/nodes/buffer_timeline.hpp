/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../private/node.hpp"
#include <map>
#include <memory>

namespace libaudioverse_implementation {

class Simulation;
class Buffer;
class BufferPlayer;

class BufferTimelineNode: public Node {
	public:
	BufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels);
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

std::shared_ptr<Node> createBufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels);
}