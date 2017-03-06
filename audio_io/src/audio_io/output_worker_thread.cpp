#include <audio_io/private/output_worker_thread.hpp>
#include <audio_io/private/logging.hpp>
#include <powercores/utilities.hpp>
#include <inttypes.h>
#include <thread>
#include <algorithm>

namespace audio_io {
namespace implementation {

OutputWorkerThread::OutputWorkerThread(std::function<void(float*, int)> callback, int sourceFrames, int sourceChannels, int sourceSr, int destinationChannels, int destinationSr, int mixahead):
converter(callback, sourceFrames, sourceChannels, sourceSr, destinationChannels, destinationSr),
prepared_buffers(mixahead),
returned_buffers(mixahead) {
	this->destination_channels = destinationChannels;
	// be careful of this cast. This can overflow without it.
	this->destination_frames = ((int64_t)sourceFrames)*destinationSr/sourceSr;
	this->mixahead = mixahead;
	int bufferSize = this->destination_channels*this->destination_frames;
	for(int i = 0; i < mixahead; i++) {
		auto buff = new AudioCommand();
		buff->data = new float[bufferSize];
		buff->type = AudioCommandType::buffer;
		while(returned_buffers.push(buff) == false);
	}
	semaphore.signal(mixahead);
	running_flag.test_and_set();
	finished_initial_mix.store(0);
	this->worker_thread = powercores::safeStartThread([&] () {workerThread();});
}

OutputWorkerThread::~OutputWorkerThread() {
	AudioCommand cmd;
	cmd.type = AudioCommandType::stop;
	while(returned_buffers.push(&cmd) == false) std::this_thread::yield();
	semaphore.signal();
	worker_thread.join();
	// Nothing else is manipulating the queues here, so dequeuing should always succeed.
	AudioCommand* tmp;
	while(returned_buffers.pop(tmp)) {
		if(tmp->type == AudioCommandType::buffer) {
			delete[] tmp->data;
		}
		delete tmp;
	}
	while(prepared_buffers.pop(tmp)) {
		if(tmp->type == AudioCommandType::buffer) {
			delete[] tmp->data;
		}
		delete tmp;
	}
}

void OutputWorkerThread::awaitInitialMix() {
	logDebug("OutputWorkerthread: initialized. Waiting on initial mix.");
	while(finished_initial_mix.load() == 0) std::this_thread::yield();
	logDebug("OutputWorkerThread: finished initial mix.");
}

void OutputWorkerThread::workerThread() {
	int mixed = 0;
	bool finishedInitialMix = false;
	while(1) {
		// the semaphore is signaled by returning buffers or a request to exit, so we wait on it.
		// It'll pause us when there's no more buffers.
		semaphore.wait();
		AudioCommand* cmd;
		while(returned_buffers.pop(cmd) == false);
		if(cmd->type == AudioCommandType::stop) return;
		cmd->used = 0;
		std::fill(cmd->data, cmd->data+destination_frames*destination_channels, 0.0f);
		converter.write(destination_frames, cmd->data);
		// If we got a buffer, there must be room to push the buffer, so this loop can't wait forever.
		while(prepared_buffers.push(cmd) == false); // Push it.
		// We need to count until we've finished the initial mixahead, then signal the constructor to continue.
		if(finishedInitialMix == false) {
			mixed += 1;
			if(mixed == mixahead) {
				finished_initial_mix.store(1); // Allow the constructor to return.
				finishedInitialMix = true;
			}
		}
	}
}

AudioCommand* OutputWorkerThread::acquireBuffer() {
	AudioCommand* buff;
	auto gotBuff = prepared_buffers.pop(buff);
	return gotBuff ? buff  : nullptr;
}

void OutputWorkerThread::releaseBuffer(AudioCommand* buff) {
	while(returned_buffers.push(buff) == false);
	semaphore.signal();
}

int OutputWorkerThread::writeHelper(int count, float* destination) {
	if(current_buffer == nullptr) {
		current_buffer = acquireBuffer();
	}
	if(current_buffer == nullptr) return 0; // because we didn't get one.
	int remaining = destination_frames-current_buffer->used;
	int willWrite = remaining < count ? remaining : count;
	float* start = current_buffer->data+destination_channels*current_buffer->used;
	float* end = start+destination_channels*willWrite;
	std::copy(start, end, destination);
	current_buffer->used += willWrite;
	if(current_buffer->used == destination_frames) {
		releaseBuffer(current_buffer);
		current_buffer = nullptr;
	}
	return willWrite;
}

int OutputWorkerThread::write(int count, float* destination) {
	int writtenTotal = 0, writtenThisTime  = 0;
	do {
		writtenThisTime = writeHelper(count, destination);
		count -= writtenThisTime;
		writtenTotal += writtenThisTime;
		destination += destination_channels*writtenThisTime;
	} while(writtenThisTime > 0);
	return writtenTotal;
}

}
}