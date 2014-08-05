/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_physical_outputs.hpp>
#include <string>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <mutex>
#include <string.h>
#include <algorithm>

/**Code common to all backends, i.e. enumeration.*/

LavPhysicalOutput::LavPhysicalOutput(unsigned int bufferSize, unsigned int mixAhead): buffer_size(bufferSize), mix_ahead(mixAhead) {
	buffers = new float*[mixAhead+1];
	buffer_statuses = new std::atomic<int>[mixAhead+1];
	for(unsigned int i = 0; i < mixAhead+1; i++) {
		buffers[i] = new float[bufferSize];
		buffer_statuses[i].store(0);
	}	
	background_thread_continue.test_and_set();
}

LavPhysicalOutput::~LavPhysicalOutput() {
	background_thread_continue.clear();
	auto ensure_stopped = std::lock_guard<std::mutex>(ensure_stopped_mutex);
	for(unsigned int i = 0; i < mix_ahead+1; i++) {
		delete[] buffers[i];
	}
	delete[] buffers;
	delete[] buffer_statuses;
}

void LavPhysicalOutput::setCallback(std::function<void(LavPhysicalOutput*, float*)> what) {
	audio_callback = what;
};

unsigned int LavPhysicalOutput::getBufferSize() {
	return buffer_size;
}

void LavPhysicalOutput::zeroOrNextBuffer(float* where) {
	if(buffer_statuses[next_buffer].load() == 1) {
		std::copy(buffers[next_buffer], buffers[next_buffer]+buffer_size, where);
		buffer_statuses[next_buffer].store(0);
	}
	else {
		memset(where, 0, sizeof(float)*buffer_size);
	}
	next_buffer += 1;
	next_buffer %= mix_ahead+1;
}
