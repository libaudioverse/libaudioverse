#pragma once
#include "semaphore.hpp"
#include "sample_format_converter.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <atomic>

namespace audio_io {
namespace implementation {

// For internal use to this class.
enum class AudioCommandType { buffer, stop };
struct AudioCommand {
	AudioCommandType type = AudioCommandType::buffer;
	float* data = nullptr;
	int used = 0;
};

/** An output worker thread.

This class knows how to write audio to a buffer provided by the consumer.  Work is done on a background thread.

Terms:
The audio thread: the thread doing the processing.
The I/O thread: the thread doing the output.

*/
class OutputWorkerThread {
	public:
	OutputWorkerThread(std::function<void(float*, int)> callback, int sourceFrames, int sourceChannels, int sourceSr, int destinationChannels, int destinationSr, int mixahead);
	~OutputWorkerThread();
	// Write count frames to destination. Return count actually written.
	// This can write partial blocks, which is functionality needed by some backends.
	// The returned value is less than count if enough data isn't available. You can use this to detect underruns by comparing.
	// use this only from the audio thread.
	int write(int count, float* destination);
	// Wait for us to have mixed enough audio to start up.
	void awaitInitialMix();
	private:
	void workerThread();
	// These are used by the I/O thread, *not* the processing thread.
	// Returns nullptr if no buffer is currently available.
	AudioCommand* acquireBuffer();
	void releaseBuffer(AudioCommand* buffer);
	int writeHelper(int count, float* destination);
	std::thread worker_thread;
	std::atomic_flag running_flag;
	std::atomic<int> finished_initial_mix;
	boost::lockfree::spsc_queue<AudioCommand*> prepared_buffers, returned_buffers;
	AudioCommand* current_buffer = nullptr;
	LightweightSemaphore semaphore;
	SampleFormatConverter converter;
	int destination_channels, destination_frames, mixahead;
};

}
}