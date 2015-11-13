/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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