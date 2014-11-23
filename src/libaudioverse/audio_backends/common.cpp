/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_audio_devices.hpp>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_resampler.hpp>
#include <string>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <mutex>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>

/**Code common to all backends, i.e. enumeration.*/

LavDevice::LavDevice(std::shared_ptr<LavSimulation> sim, unsigned int mixAhead): mix_ahead(mixAhead), simulation(sim) {
	buffer_statuses = new std::atomic<int>[mixAhead+1];
	for(unsigned int i = 0; i < mixAhead + 1; i++) buffer_statuses[i].store(0);
}

//these are the next two steps in initialization, and are consequently put before the destructor.
void LavDevice::init(unsigned int targetSr, unsigned int channels) {
	this->channels = channels;
	target_sr = targetSr;
	//compute an estimated "good" size for the buffers, given the device's blockSize and channels.
	output_buffer_size = (unsigned int)simulation->getBlockSize()*channels;
	if(targetSr != simulation->getSr()) {
		output_buffer_size = (unsigned int)(output_buffer_size*(double)targetSr/simulation->getSr());
		//always go for the multiples of 4.  This doesn't hurt anything, and some backends may be faster because of it.
		if(output_buffer_size%4) output_buffer_size = output_buffer_size+(4-output_buffer_size%4);
	}
	unsigned int goodBufferSize = output_buffer_size*channels;
	buffers = new float*[mix_ahead];
	for(unsigned int i = 0; i < mix_ahead+1; i++) {
		buffers[i] = new float[goodBufferSize];
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
	unsigned int sourceSr = (unsigned int)simulation->getSr();
	LavResampler resampler((unsigned int)simulation->getBlockSize(), channels, sourceSr, target_sr);
	unsigned int currentBuffer = 0;
	unsigned int sleepFor = (unsigned int)((simulation->getBlockSize()/(double)simulation->getSr())*1000);
	float* tempBuffer = new float[simulation->getBlockSize()*channels]();
	while(mixing_thread_continue.test_and_set()) {
		if(buffer_statuses[currentBuffer].load()) { //we've done this one, but the callback hasn't gotten to it yet.
			if(hasFilledQueueFirstTime == false) {
				startup_hook();
				hasFilledQueueFirstTime = true;
			}
			if(sleepFor) std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
			continue;
		}
		if(sourceSr == target_sr) {
			simulation->lock();
			simulation->getBlock(buffers[currentBuffer], channels);
			simulation->unlock();
		}
		else { //we need to resample.
			unsigned int got = 0;
			simulation->lock();
			while(got < output_buffer_size) {
				simulation->getBlock(tempBuffer, channels);
				resampler.read(tempBuffer);
				got += resampler.write(buffers[currentBuffer]+got, output_buffer_size-got);
			}
			simulation->unlock();
		}
		buffer_statuses[currentBuffer].store(1); //mark it as ready.
		currentBuffer ++;
		currentBuffer %= mix_ahead+1;
	}
	shutdown_hook();
}

unsigned int LavSimulationFactory::getOutputCount() {
	return (unsigned int)output_count;
}

std::string LavSimulationFactory::getName() {
	return "Invalid backend: subclass failed to implement";
}
