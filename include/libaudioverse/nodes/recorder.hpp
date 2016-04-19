/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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