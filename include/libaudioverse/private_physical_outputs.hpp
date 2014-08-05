/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_sourcemanager.hpp"
#include <vector>
#include <set>
#include <memory>
#include <utility>
#include <atomic>
#include <mutex>

/**A physical output.*/
class LavPhysicalOutput {
	protected:
	LavPhysicalOutput(unsigned int bufferSize, unsigned int mixAhead);
	virtual ~LavPhysicalOutput();
	virtual void zeroOrNextBuffer(float* where);
	unsigned int buffer_size = 0, mix_ahead = 0;
	unsigned int next_output_buffer = 0;
	float** buffers = nullptr;
	std::atomic<int>* buffer_statuses = nullptr;
	friend class LavPhysicalOutputFactory;
	std::atomic_flag background_thread_continue;
	std::mutex ensure_stopped_mutex; //held by the background thread as long as that thread is running.
};

class LavPhysicalOutputFactory {
	public:
	LavPhysicalOutputFactory() = delete;
	virtual ~LavPhysicalOutputFactory() {}
	virtual std::vector<std::string> getOutputNames() = 0;
	virtual std::vector<float> getOutputLatencies() = 0;
	virtual std::vector<int> getOutputMaxChannels() = 0;
	virtual void rescan();
};

bool portaudioBackendAvailable();
LavPhysicalOutputFactory* createPortaudioPhysicalOutputFactory();
