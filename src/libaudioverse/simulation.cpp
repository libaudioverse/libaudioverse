/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/connections.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/data.hpp>
#include <libaudioverse/private/file.hpp>
#include <libaudioverse/private/planner.hpp>
#include <libaudioverse/private/logging.hpp>
#include <libaudioverse/private/helper_templates.hpp>
#include <powercores/utilities.hpp>
#include <audio_io/audio_io.hpp>
#include <stdlib.h>
#include <functional>
#include <algorithm>
#include <iterator>
#include <thread>
#include <tuple>
#include <map>

namespace libaudioverse_implementation {

Simulation::Simulation(unsigned int sr, unsigned int blockSize, unsigned int mixahead): Job(Lav_OBJTYPE_SIMULATION) {
	if(blockSize%4 || blockSize== 0) ERROR(Lav_ERROR_RANGE, "Block size must be a nonzero multiple of 4."); //only afe to have this be a multiple of four.
	this->sr = (float)sr;
	this->block_size = blockSize;
	this->mixahead = mixahead;
	//fire up the background thread.
	backgroundTaskThread = powercores::safeStartThread(&Simulation::backgroundTaskThreadFunction, this);
	planner = new Planner();
	//Get thread count.
	int defaultThreadCount = std::thread::hardware_concurrency();
	if(defaultThreadCount == 0) {
		logInfo("Simulation: threading implementation does not support querying hardware concurrency.");
		defaultThreadCount = 1;
	}
	if(defaultThreadCount > 1) logInfo("Simulation: enabling concurrency with %i threads.", defaultThreadCount);
	else logInfo("Simulation: not enabling concurrency.  CPU only supports one thread.");
	setThreads(defaultThreadCount);
	start();
}

void Simulation::completeInitialization() {
	final_output_connection =std::make_shared<InputConnection>(std::static_pointer_cast<Simulation>(this->shared_from_this()), nullptr, 0, 0);
}

Simulation::~Simulation() {
	//enqueue a task which will stop the background thread.
	enqueueTask([]() {throw ThreadTerminationException();});
	backgroundTaskThread.join();
	delete planner;
}

//Yes, this uses goto. Yes, goto is evil. We need a single point of exit.
void Simulation::getBlock(float* out, unsigned int channels, bool mayApplyMixingMatrix) {
	if(out == nullptr || channels == 0) {
		memset(out, 0, sizeof(float)*channels*block_size);
		goto end;
	}
	if(block_callback) block_callback(outgoingObject(this->shared_from_this()), getCurrentTime()-block_callback_set_time, block_callback_userdata);
	//configure our connection to the number of channels requested.
	final_output_connection->reconfigure(0, channels);
	//append buffers to the final_outputs vector until it's big enough.
	//in a sane application, we'll never go above 8 channels so keeping them around is no big deal.
	while(final_outputs.size() < channels) final_outputs.push_back(allocArray<float>(block_size));
	//zero the outputs we need.
	for(unsigned int i= 0; i < channels; i++) memset(final_outputs[i], 0, sizeof(float)*block_size);
	//Inform nodes that we are going to tick.
	for(auto &i: will_tick_nodes) {
		auto n = i.lock();
		if(n) n->willTick();
	}
	//Use the planner.
	planner->execute(std::dynamic_pointer_cast<Job>(shared_from_this()), threads);
	//write, applying mixing matrices as needed.
	final_output_connection->addNodeless(&final_outputs[0], true);
	//interleave the samples.
	interleaveSamples(channels, block_size, channels, &final_outputs[0], out);
	end:
	time +=block_size/sr;
	int maintenance_count=maintenance_start;
	filterWeakPointers(maintenance_nodes, [&](std::shared_ptr<Node> &i_s) {
		if(maintenance_count % maintenance_rate== 0) i_s->doMaintenance();
		maintenance_count++;
	});
	maintenance_start++;
	//and ourselves.
	if(maintenance_start%maintenance_rate == 0) doMaintenance();
	tick_count ++;
	//Finally, we have to call any scheduled callbacks for this block.
	filter(scheduled_callbacks, [&](auto item, double t) {
		if(t >= item.first) {
			item.second();
			return false; //to kill.
		}
		else return true; //to keep.
	}, getCurrentTime());
}

void Simulation::doMaintenance() {
	killDeadWeakPointers(nodes);
	killDeadWeakPointers(will_tick_nodes);
}

void Simulation::setOutputDevice(int index, int channels) {
	if(index < -1) ERROR(Lav_ERROR_RANGE, "Index -1 is default; all other negative numbers are invalid.");
	if(output_device) {
		output_device->stop();
	}
	std::lock_guard<std::recursive_mutex> g(mutex);
	auto &factory = getOutputDeviceFactory();
	if(factory == nullptr) ERROR(Lav_ERROR_CANNOT_INIT_AUDIO, "Failed to get output device factory.");
	auto sptr = std::static_pointer_cast<Simulation>(shared_from_this());
	std::weak_ptr<Simulation> wptr(sptr);
	int blockSize=getBlockSize();
	auto cb =[wptr, blockSize](float* buffer, int channels)->void {
		auto strong =wptr.lock();
		if(strong==nullptr) memset(buffer, 0, sizeof(float)*blockSize*channels);
		else {
			std::lock_guard<Simulation> guard(*strong);
			strong->getBlock(buffer, channels);
		}
	};
	try {
		output_device =factory->createDevice(cb, index, channels, getSr(), getBlockSize(), 0.0, 0.1, 0.2);
		if(output_device == nullptr) ERROR(Lav_ERROR_CANNOT_INIT_AUDIO, "Device could not be created.");
	}
	catch(std::exception &e) {
		ERROR(Lav_ERROR_CANNOT_INIT_AUDIO, e.what());
	}
}

void Simulation::clearOutputDevice() {
	output_device=nullptr;
}

std::shared_ptr<InputConnection> Simulation::getFinalOutputConnection() {
	return final_output_connection;
}

LavError Simulation::start() {
	is_started = 1;
	return Lav_ERROR_NONE;
}

LavError Simulation::stop() {
	is_started = 0;
	return Lav_ERROR_NONE;
}

void Simulation::associateNode(std::shared_ptr<Node> node) {
	nodes.insert(std::weak_ptr<Node>(node));
}

void Simulation::registerNodeForWillTick(std::shared_ptr<Node> node) {
	will_tick_nodes.insert(node);
}

void Simulation::registerNodeForAlwaysPlaying(std::shared_ptr<Node> which) {
	always_playing_nodes.insert(which);
}

void Simulation::unregisterNodeForAlwaysPlaying(std::shared_ptr<Node> which) {
	always_playing_nodes.erase(which);
}

void Simulation::registerNodeForMaintenance(std::shared_ptr<Node> which) {
	maintenance_nodes.insert(which);
}

void Simulation::enqueueTask(std::function<void(void)> cb) {
	tasks.enqueue(cb);
}

//Default callback implementation.
void Simulation::backgroundTaskThreadFunction() {
	try {
		for(;;) {
			auto task = tasks.dequeue();
			task();
		}
	}
	catch(ThreadTerminationException) {
		return;
	}
}

void Simulation::setBlockCallback(LavTimeCallback callback, void* userdata) {
	block_callback = callback;
	block_callback_set_time = getCurrentTime();
	block_callback_userdata=userdata;
}

void Simulation::writeFile(std::string path, int channels, double duration, bool mayApplyMixingMatrix) {
	int blocks = (duration*getSr()/block_size)+1;
	auto file = FileWriter();
	file.open(path.c_str(), sr, channels);
	//vectors are guaranteed to be contiguous. Using a vector here guarantees proper cleanup.
	std::vector<float> block;
	block.resize(channels*getBlockSize());
	for(int i = 0; i < blocks; i++) {
		getBlock(&block[0], channels, mayApplyMixingMatrix);
		unsigned int written= 0;
		while(written < getBlockSize()) written += file.write(getBlockSize(), &block[0]);
	}
	file.close();
}

void Simulation::setThreads(int n) {
	threads = n;
}

int Simulation::getThreads() {
	return threads;
}

void Simulation::invalidatePlan() {
	planner->invalidatePlan();
}

double Simulation::getCurrentTime() {
	return time;
}

void Simulation::scheduleCall(double when, std::function<void(void)> func) {
	scheduled_callbacks.insert(std::make_pair(getCurrentTime()+when, func));
}

//begin public API

Lav_PUBLIC_FUNCTION LavError Lav_createSimulation(unsigned int sr, unsigned int blockSize, LavHandle* destination) {
	PUB_BEGIN
	auto shared = std::make_shared<Simulation>(sr, blockSize, 0);
	shared->completeInitialization();
	*destination = outgoingObject(shared);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlock(LavHandle simulationHandle, unsigned int channels, int mayApplyMixingMatrix, float* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	simulation->getBlock(destination, channels, mayApplyMixingMatrix != 0);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlockSize(LavHandle simulationHandle, int* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = simulation->getBlockSize();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetSr(LavHandle simulationHandle, int* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = (int)simulation->getSr();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationSetOutputDevice(LavHandle simulationHandle, const char* device, int channels) {
	PUB_BEGIN
	auto sim = incomingObject<Simulation>(simulationHandle);
	int index;
	auto device_string = std::string(device);
	if(device_string == "default") {
		index = -1;
	}
	else {
		size_t processed;
		try {
			index = std::stoi(device_string, &processed, 10);
		}
		catch(...) {
			ERROR(Lav_ERROR_NO_SUCH_DEVICE, "No such device.");
		}
		if(processed == 0) {ERROR(Lav_ERROR_NO_SUCH_DEVICE, "Identifier string is invalid.");}
	}
	//This is threadsafe and needs to be entered properly so it can make sure we dont' deadlock in audio_io.
	sim->setOutputDevice(index, channels);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationClearOutputDevice(LavHandle simulationHandle) {
	PUB_BEGIN
	auto sim = incomingObject<Simulation>(simulationHandle);
	LOCK(*sim);
	sim->clearOutputDevice();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationLock(LavHandle simulationHandle) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	simulation->lock();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationUnlock(LavHandle simulationHandle) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	simulation->unlock();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationSetBlockCallback(LavHandle handle, LavTimeCallback callback, void* userdata) {
	PUB_BEGIN
	incomingObject<Simulation>(handle)->setBlockCallback(callback, userdata);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationWriteFile(LavHandle simulationHandle, const char* path, int channels, double duration, int mayApplyMixingMatrix) {
	PUB_BEGIN
	auto sim = incomingObject<Simulation>(simulationHandle);
	LOCK(*sim);
	sim->writeFile(path, channels, duration, mayApplyMixingMatrix);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationSetThreads(LavHandle simulationHandle, int threads) {
	PUB_BEGIN
	if(threads < 1) ERROR(Lav_ERROR_RANGE, "Cannot run simulation with less than one thread.");
	auto sim = incomingObject<Simulation>(simulationHandle);
	LOCK(*sim);
	sim->setThreads(threads);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetThreads(LavHandle simulationHandle, int* destination) {
	PUB_BEGIN
	auto sim = incomingObject<Simulation>(simulationHandle);
	LOCK(*sim);
	*destination = sim->getThreads();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationCallIn(LavHandle simulationHandle, double when, int inAudioThread, LavTimeCallback cb, void* userdata) {
	PUB_BEGIN
	auto sim = incomingObject<Simulation>(simulationHandle);
	std::weak_ptr<Simulation> simWeak = sim;
	auto wrapped_callback = [simWeak, userdata, cb] () {
		auto simStrong = simWeak.lock();
		//This should always be a valid weak pointer, but we check here just in case.
		if(simStrong) {
			double t = simStrong->getCurrentTime();
			cb(outgoingObject(simStrong), t, userdata);
		}
	};
	LOCK(*sim);
	if(inAudioThread) sim->scheduleCallInAudioThread(when, wrapped_callback);
	else sim->scheduleCallOutsideAudioThread(when, wrapped_callback);
	PUB_END
}

}