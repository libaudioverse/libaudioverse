/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/resampler.hpp>
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/private/errors.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <utility>
#include <mutex>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>

/**Code common to all backends, i.e. enumeration.*/

//these are the two steps in initialization, and are consequently put before the destructor.
void LavDevice::init(std::function<void(float*, int)> getBuffer, unsigned int inputBufferFrames, unsigned int inputBufferSr, unsigned int channels, unsigned int outputSr, unsigned int mixAhead) {
	input_buffer_frames = inputBufferFrames;
	mix_ahead = mixAhead;
	input_buffer_size = inputBufferFrames*channels;
	input_sr = inputBufferSr;
	output_sr = outputSr;
	this->channels = channels;
	buffer_statuses = new std::atomic<int>[mixAhead+1];
	get_buffer = getBuffer;
	for(unsigned int i = 0; i < mixAhead + 1; i++) buffer_statuses[i].store(0);
	if(input_sr != output_sr) is_resampling = true;
	output_buffer_frames = input_buffer_frames;
	if(output_sr != input_sr) {
		output_buffer_frames = (unsigned int)(output_buffer_frames*(double)output_sr)/input_sr;
	}
	output_buffer_size = output_buffer_frames*channels;
	buffers = new float*[mix_ahead];
	for(unsigned int i = 0; i < mix_ahead+1; i++) {
		buffers[i] = new float[output_buffer_size];
	}
}

void LavDevice::start() {
	mixing_thread_continue.test_and_set();
	mixing_thread = std::thread([this] () {mixingThreadFunction();});
	started = true;
}

LavDevice::~LavDevice() {
	stop();
	if(buffers != nullptr)
	for(unsigned int i = 0; i < mix_ahead+1; i++) {
		if(buffers[i]) delete[] buffers[i];
	}
	if(buffers) delete[] buffers;
	if(buffer_statuses) delete[] buffer_statuses;
}

void LavDevice::stop() {
	if(started == false) return;
	mixing_thread_continue.clear();
	mixing_thread.join();
}

void LavDevice::zeroOrNextBuffer(float* where) {
	if(buffer_statuses[next_output_buffer].load() == 1) {
		std::copy(buffers[next_output_buffer], buffers[next_output_buffer]+output_buffer_size, where);
		buffer_statuses[next_output_buffer].store(0);
	}
	else {
		memset(where, 0, sizeof(float)*output_buffer_size);
	}
	next_output_buffer += 1;
	next_output_buffer %= mix_ahead+1;
}

void LavDevice::startup_hook() {
}

void LavDevice::shutdown_hook() {
}

void LavDevice::mixingThreadFunction() {
	bool hasFilledQueueFirstTime = false;
	LavResampler resampler(input_buffer_frames, channels, input_sr, output_sr);
	unsigned int currentBuffer = 0;
	unsigned int sleepFor = (unsigned int)(((double)input_buffer_frames/input_sr)*1000);
	float* currentBlock = new float[input_buffer_size]();
	float* resampledBlock= new float[output_buffer_frames*channels]();
	while(mixing_thread_continue.test_and_set()) {
		if(buffer_statuses[currentBuffer].load()) { //we've done this one, but the callback hasn't gotten to it yet.
			if(hasFilledQueueFirstTime == false) {
				startup_hook();
				hasFilledQueueFirstTime = true;
			}
			if(sleepFor) std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
			continue;
		}
		if(is_resampling == false) {
			get_buffer(currentBlock, channels);
		}
		else { //we need to resample.
			unsigned int got = 0;
			while(got < output_buffer_frames) {
				get_buffer(currentBlock, channels);
				resampler.read(currentBlock);
				got += resampler.write(resampledBlock+got, output_buffer_frames-got);
			}
		}
		if(is_resampling == false) {
			std::copy(currentBlock, currentBlock+output_buffer_size, buffers[currentBuffer]);
		}
		else {
			std::copy(resampledBlock, resampledBlock+output_buffer_size, buffers[currentBuffer]);
		}
		buffer_statuses[currentBuffer].store(1); //mark it as ready.
		currentBuffer ++;
		currentBuffer %= mix_ahead+1;
	}
	shutdown_hook();
}

unsigned int LavDeviceFactory::getOutputCount() {
	return (unsigned int)output_count;
}

std::string LavDeviceFactory::getName() {
	return "Invalid backend: subclass failed to implement";
}
