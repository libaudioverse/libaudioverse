/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../private/file.hpp"
#include <powercores/threadsafe_queue.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

namespace libaudioverse_implementation {

class Simulation;

class RecorderNode: public Node {
	public:
	RecorderNode(std::shared_ptr<Simulation> simulation, int channels);
	~RecorderNode();
	void recordingThreadFunction();
	virtual void process() override;
	virtual void doMaintenance() override;
	void startRecording(std::string path);
	void stopRecording();
	std::thread recording_thread;
	powercores::ThreadsafeQueue<float*> available_buffers;
	powercores::ThreadsafeQueue<float*> unwritten_buffers;
	int buffer_size = 0;
	int buffer_freeing_threshold =20; //If we ever get this far ahead, we free some buffers.
	std::atomic_flag should_keep_recording;
	FileWriter recording_to;
	int channels = 0;
	bool recording = false;
};

std::shared_ptr<Node> createRecorderNode(std::shared_ptr<Simulation> simulation, int channels);
}