/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include <vector>
#include <set>
#include <memory>
#include <utility>
#include <atomic>
#include <mutex>
#include <thread>
#include <functional>

namespace libaudioverse_implementation {

/**A physical output.*/
class Device {
	protected:
	Device() = default;
	virtual ~Device();
	//function parameters: output buffer, number of channels to write.
	virtual void init(std::function<void(float*, int)> getBuffer, unsigned int inputBufferFrames, unsigned int inputBufferSr, unsigned int channels, unsigned int outputSr, unsigned int mixAhead); //second step fn initialization. We can't just fall through to the constructor.
	virtual void start(); //final step in initialization via subclasses: starts the background thread.
	virtual void stop(); //stop the output.
	//these hooks are run in the background thread, and should be overridden in subclasses.
	virtual void startup_hook();
	virtual void shutdown_hook();
	virtual void zeroOrNextBuffer(float* where);
	virtual void mixingThreadFunction();
	unsigned int channels = 0;
	unsigned int mix_ahead = 0;
	unsigned int input_buffer_size, output_buffer_size, input_buffer_frames, output_buffer_frames;
	unsigned int input_sr = 0;
	unsigned int output_sr = 0;
	bool is_resampling = false;
	bool should_apply_mixing_matrix = false;
	unsigned int next_output_buffer = 0;
	unsigned int callback_buffer_index = 0;

	float** buffers = nullptr;
	std::atomic<int>* buffer_statuses = nullptr;
	std::atomic_flag mixing_thread_continue;
	std::thread mixing_thread;
	std::function<void(float*, int)> get_buffer;
	bool started = false;
	friend class DeviceFactory;
};

class DeviceFactory {
	public:
	DeviceFactory() = default;
	virtual ~DeviceFactory();
	virtual std::vector<std::string> getOutputNames() = 0;
	virtual std::vector<int> getOutputMaxChannels() = 0;

	virtual std::shared_ptr<Device> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) = 0;
	virtual unsigned int getOutputCount();
	virtual std::string getName();
	protected:
	std::vector<std::weak_ptr<Device>> created_devices;
	int output_count = 0;
};

typedef DeviceFactory* (*DeviceFactoryCreationFunction)();
DeviceFactory* createWinmmDeviceFactory();
DeviceFactory* createOpenALDeviceFactory();

//finally, the function that initializes all of this.
void initializeDeviceFactory();
void shutdownDeviceFactory();

}