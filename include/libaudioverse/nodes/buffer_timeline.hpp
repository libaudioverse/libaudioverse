/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include <vector>
#include <memory>

namespace libaudioverse_implementation {

class Simulation;
class Buffer;

class ScheduledBuffer {
	public:
	ScheduledBuffer(std::shared_ptr<Buffer> buffer, double time, float delta, int outputChannels);
	bool add(float* destination, int maxFrames, int channel); //returns true when there is no more to write.
	std::shared_ptr<Buffer> buffer;
	int length = 0, output_channels = 0, buffer_channels = 0;
	std::vector<int> position;
	float offset = 0.0f;
	double time;
	float delta = 1.0;
};

class BufferTimelineNode: public Node {
	public:
	BufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels);
	void process() override;
	void scheduleBuffer(double time, float delta, std::shared_ptr<Buffer> buffer);
	void reset() override;
	private:
	std::vector<ScheduledBuffer> scheduled_buffers;
	std::vector<ScheduledBuffer> active_buffers;
	double time = 0.0;
	int output_channels = 0;
};

std::shared_ptr<Node> createBufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels);
}