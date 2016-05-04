/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include <audio_io/audio_io.hpp>
#include <powercores/threadsafe_queue.hpp>
#include <functional> //we have to use an std::function for the preprocessing hook.  There's no good way around it because worlds need to use capturing lambdas.
#include <set>
#include <vector>
#include <mutex>
#include <memory>
#include <thread>
#include <tuple>
#include <map>
#include <random>
#include "../libaudioverse.h"
#include "memory.hpp"
#include "job.hpp"

namespace libaudioverse_implementation {

class Node;
class Device;
class InputConnection;
class Planner;

/*When thrown on the background thread, terminates it.*/
class ThreadTerminationException {
};

class Simulation: public Job {
	public:
	Simulation(unsigned int sr, unsigned int blockSize, unsigned int mixahead);
	//needed because the InputConnection needs us to use shared_from_this.
	void completeInitialization();
	~Simulation();
	void getBlock(float* out, unsigned int channels, bool mayApplyMixingMatrix = true);
	std::shared_ptr<InputConnection> getFinalOutputConnection();

	//this is in frames of audio data.
	unsigned int getBlockSize() { return block_size;}
	LavError start();
	LavError stop();
	void associateNode(std::shared_ptr<Node> node);
	//Indicates that a node should have willTick called on it.
	void registerNodeForWillTick(std::shared_ptr<Node> node);
	//used to register and unregister for always playing status.
	void registerNodeForAlwaysPlaying(std::shared_ptr<Node> which);
	void unregisterNodeForAlwaysPlaying(std::shared_ptr<Node> which);
	//If we need maintenance.
	void registerNodeForMaintenance(std::shared_ptr<Node> which);
	
	float getSr() { return sr;}
	int getTickCount() {return tick_count;}
	void doMaintenance(); //cleans up dead weak pointers, etc.
	//these make us meet the basic lockable concept.
	void lock() {mutex.lock();}
	void unlock() {mutex.unlock();}

	//associate with the specified device index.
	//This must absolutely absolutely absolutely be called without the lock, it's threadsafe.
	void setOutputDevice(int index, int channels);
	void clearOutputDevice();

	//Tasks that need to run in the background.
	void enqueueTask(std::function<void(void)>);
	//Set the block callback.
	void setBlockCallback(LavTimeCallback cb, void* userdata);

	//Write to a file.
	void writeFile(std::string path, int channels, double duration, bool mayApplyMixingMatrix);

	//Thread support.
	void setThreads(int n);
	int getThreads();

	//called when connections are formed or lost, or when a node is deleted.
	void invalidatePlan();
	
	//Get the time. This is relative to whenever the simulation was created, and advances with getBlock.
	double getCurrentTime();
	
	//Schedule something to run in a while, inside the audio thread.
	//when is relative to now.
	template<typename CallableT, typename... ArgsT>
	void scheduleCallInAudioThread(double when, CallableT callable, ArgsT... args) {
		scheduleCall(when, [=] () {callable(args...);});
	}
	
	//Like callAfterInAudioThread, but for things outside the audio thread.
	template<typename CallableT, typename... ArgsT>
	void scheduleCallOutsideAudioThread(double when, CallableT callable, ArgsT... args) {
		auto c = [=] () {
			callable(args...);
		};
		scheduleCall(when, c);
	}
	
	protected:
	//Schedule a call in when seconds.
	void scheduleCall(double when, std::function<void(void)> func);
	
	//the connection to which nodes connect themselves if their output should be audible.
	std::shared_ptr<InputConnection> final_output_connection;
	//pointers to output buffers that the above connection can write to.
	std::vector<float*> final_outputs;

	unsigned int block_size = 0, mixahead = 0, is_started = 0;
	float sr = 0.0f;
	//if nodes die, they automatically need to be removed.  We can do said removal on next process.
	std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> nodes;
	std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> will_tick_nodes; //Nodes to call willTick on.
	std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> always_playing_nodes; //Nodes that are currently always playing.
	std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> maintenance_nodes; //Nodes that need doMaintenance.
	
	std::recursive_mutex mutex;

	powercores::ThreadsafeQueue<std::function<void(void)>>  tasks;
	std::thread backgroundTaskThread;
	void backgroundTaskThreadFunction();

	//our output, if any.
	std::unique_ptr<audio_io::OutputDevice> output_device = nullptr;

	int tick_count = 0; //counts ticks.  This is part of node processing.
	int maintenance_start = 0; //also part of node processing. Used to stagger calls to doMaintenance on nodes so that we're not randomly spiking the tick length.
	int maintenance_rate = 5; //call on every 5th object.

	//support for callbacks and time.
	double time = 0.0;
	LavTimeCallback block_callback = nullptr;
	void* block_callback_userdata =nullptr;
	double block_callback_set_time = 0.0;
	std::multimap<double, std::function<void(void)>> scheduled_callbacks;
	
	Planner* planner = nullptr;
	int threads = 1;
	
	template<typename JobT, typename CallableT, typename... ArgsT>
	friend void simulationVisitDependencies(JobT&& start, CallableT&& callable, ArgsT&&... args);
};

}