/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/constants.hpp>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <powercores/threadsafe_queue.hpp>
#include <powercores/exceptions.hpp>
#include <powercores/utilities.hpp>
#include <thread>
#include <mutex>
#include <atomic>

namespace libaudioverse_implementation {

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

RecorderNode::RecorderNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_RECORDER_NODE, simulation, channels, channels) {
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	this->channels = channels;
	buffer_size=simulation->getBlockSize()*channels;
	//allocate a few buffers.
	for(int i=0; i < 5; i++) available_buffers.enqueue(allocArray<float>(buffer_size));
}

std::shared_ptr<Node> createRecorderNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<RecorderNode>(simulation, channels);
}

RecorderNode::~RecorderNode() {
	stopRecording();
	while(available_buffers.size()) freeArray(available_buffers.dequeue());
	while(unwritten_buffers.size()) freeArray(unwritten_buffers.dequeue());
}

void RecorderNode::recordingThreadFunction() {
	try { //If we get an error that gets all the way out to here, we need to abort the thread without terminating the ap.
		while(should_keep_recording.test_and_set()) {
			float* buffer= nullptr;
			try {
				buffer = unwritten_buffers.dequeueWithTimeout(5);
			}
			catch(powercores::TimeoutException e) {
				continue; //try again in  a bit, if no one has cleared the flag.
			}
			int frames= 0;
			while(frames != block_size) frames += recording_to.write(block_size-frames, buffer+frames*channels);
			available_buffers.enqueue(buffer);
		}
	}
	catch(...) {
		//Todo: log here.
		//We do this because in theory an open file can be invalidated, and the app should keep running.
	}
}

void RecorderNode::process() {
	if(recording) {
		float* buffer;
		if(available_buffers.size()) buffer = available_buffers.dequeue();
		else buffer =allocArray<float>(buffer_size);
		interleaveSamples(num_input_buffers, block_size, num_input_buffers, &input_buffers[0], buffer);
		unwritten_buffers.enqueue(buffer);
	}
	for(int i = 0; i < num_output_buffers; i++) std::copy(input_buffers[i], input_buffers[i]+block_size, output_buffers[i]);
}

void RecorderNode::doMaintenance() {
	while(available_buffers.size() > buffer_freeing_threshold) freeArray(available_buffers.dequeue());
}

void RecorderNode::startRecording(std::string path) {
	if(recording) stopRecording();
	recording_to.open(path.c_str(), simulation->getSr(), channels);
	recording = true;
	should_keep_recording.test_and_set(); //so the thread doesn't immediately stop.
	recording_thread= powercores::safeStartThread(&RecorderNode::recordingThreadFunction, this);
}

void RecorderNode::stopRecording() {
	if(recording) {
		should_keep_recording.clear();
		recording_thread.join();
		recording_to.close();
		recording =false;
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createRecorderNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createRecorderNode(simulation, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_recorderNodeStartRecording(LavHandle nodeHandle, const char* path) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	if(node->getType() != Lav_OBJTYPE_RECORDER_NODE) ERROR(Lav_ERROR_TYPE_MISMATCH, "Expected a recorder node.");
	auto recorder=std::static_pointer_cast<RecorderNode>(node);
	LOCK(*recorder);
	recorder->startRecording(path);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_recorderNodeStopRecording(LavHandle nodeHandle) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	if(node->getType() != Lav_OBJTYPE_RECORDER_NODE) ERROR(Lav_ERROR_TYPE_MISMATCH);
	auto recorder=std::static_pointer_cast<RecorderNode>(node);
	LOCK(*recorder);
	recorder->stopRecording();
	PUB_END
}

}