/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../private/node.hpp"
#include "../private/file.hpp"
#include <powercores/threadsafe_queue.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

namespace libaudioverse_implementation {

class Server;

class RecorderNode: public Node {
	public:
	RecorderNode(std::shared_ptr<Server> server, int channels);
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

std::shared_ptr<Node> createRecorderNode(std::shared_ptr<Server> server, int channels);
}