/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_physical_outputs.hpp>
#include <libaudioverse/private_devices.hpp>
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

LavPhysicalOutput::LavPhysicalOutput(std::shared_ptr<LavDevice> dev, unsigned int mixAhead): mix_ahead(mixAhead), device(device), channels(device->getChannels()) {
	buffers = new float*[mixAhead+1];
	buffer_statuses = new std::atomic<int>[mixAhead+1];
}

//these are the next two steps in initialization, and are consequently put before the destructor.
void LavPhysicalOutput::init(unsigned int targetSr) {
	target_sr = targetSr;
	//compute an estimated "good" size for the buffers, given the device's blockSize and channels.
	output_buffer_size = (unsigned int)device->getBlockSize();
	output_buffer_size = (unsigned int)(output_buffer_size*(double)device->getSr()/targetSr);
	//always go for the multiples of 4.  This doesn't hurt anything, and some backends may be faster because of it.
	if(output_buffer_size%4) output_buffer_size = output_buffer_size+(4-output_buffer_size%4);
	unsigned int goodBufferSize = output_buffer_size*channels;
	for(unsigned int i = 0; i < mix_ahead+1; i++) {
		buffers[i] = new float[goodBufferSize];
	}
}

void LavPhysicalOutput::start() {
	mixing_thread_continue.test_and_set();
	mixing_thread = std::thread([this] () {mixingThreadFunction();});
}

LavPhysicalOutput::~LavPhysicalOutput() {
	stop();
	for(unsigned int i = 0; i < mix_ahead+1; i++) {
		delete[] buffers[i];
	}
	delete[] buffers;
	delete[] buffer_statuses;
}

void LavPhysicalOutput::stop() {
	mixing_thread_continue.clear();
	mixing_thread.join();
}

void LavPhysicalOutput::zeroOrNextBuffer(float* where) {
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

void LavPhysicalOutput::mixingThreadFunction() {
	unsigned int sourceSr = (unsigned int)device->getSr();
	LavResampler resampler((unsigned int)device->getBlockSize(), device->getChannels(), sourceSr, target_sr);
	unsigned int currentBuffer = 0;
	unsigned int sleepFor = (unsigned int)(device->getBlockSize()/(double)device->getSr())*1000;
	float* tempBuffer = new float[device->getBlockSize()*device->getChannels()]();
	while(mixing_thread_continue.test_and_set()) {
		if(buffer_statuses[currentBuffer].load()) { //we've done this one, but the callback hasn't gotten to it yet.
			if(sleepFor) std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
			continue;
		}
		if(sourceSr == target_sr) {
			device->lock();
			device->getBlock(buffers[currentBuffer]);
			device->unlock();
		}
		else { //we need to resample.
			unsigned int got = 0;
			device->lock();
			while(got < output_buffer_size) {
				device->getBlock(tempBuffer);
				resampler.read(tempBuffer);
				got += resampler.write(buffers[currentBuffer]+got, output_buffer_size-got);
			}
			device->unlock();
		}
		buffer_statuses[currentBuffer].store(1); //mark it as ready.
		currentBuffer ++;
		currentBuffer %= mix_ahead+1;
	}
}
