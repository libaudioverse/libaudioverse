/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
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

namespace libaudioverse_implementation {

class Node;
class Device;
class InputConnection;

/*When thrown on the background thread, terminates it.*/
class ThreadTerminationException {
};

class Simulation: public ExternalObject {
	public:
	Simulation(unsigned int sr, unsigned int blockSize, unsigned int mixahead);
	//needed because the InputConnection needs us to use shared_from_this.
	void completeInitialization();
	virtual ~Simulation();
	virtual void getBlock(float* out, unsigned int channels, bool mayApplyMixingMatrix = true);
	std::shared_ptr<InputConnection> getFinalOutputConnection();

	//this is in frames of audio data.
	virtual unsigned int getBlockSize() { return block_size;}
	virtual LavError start();
	virtual LavError stop();
	virtual LavError associateNode(std::shared_ptr<Node> node);

	virtual float getSr() { return sr;}
	virtual int getTickCount() {return tick_count;}
	virtual void doMaintenance(); //cleans up dead weak pointers, etc.
	//these make us meet the basic lockable concept.
	void lock() {mutex.lock();}
	void unlock() {mutex.unlock();}

	//Tasks that need to run in the background.
	void enqueueTask(std::function<void(void)>);

	//makes this device hold a shared pointer to its output.
	void associateDevice(std::shared_ptr<Device> what);

	//register a mixing matrix with this device.
	void registerMixingMatrix(unsigned int inChannels, unsigned int outChannels, float* matrix);
	void resetMixingMatrix(unsigned int inChannels, unsigned int outChannels);
	void registerDefaultMixingMatrices();
	const float* getMixingMatrix(unsigned int inChannels, unsigned int outChannels);

	//Set the block callback.
	virtual void setBlockCallback(LavBlockCallback cb, void* userdata);

	protected:
	//the connection to which nodes connect themselves if their output should be audible.
	std::shared_ptr<InputConnection> final_output_connection;
	//pointers to output buffers that the above connection can write to.
	std::vector<float*> final_outputs;

	unsigned int block_size = 0, mixahead = 0, is_started = 0;
	float sr = 0.0f;
	//if nodes die, they automatically need to be removed.  We can do said removal on next process.
	std::set<std::weak_ptr<Node>, std::owner_less<std::weak_ptr<Node>>> nodes;
	std::recursive_mutex mutex;

	powercores::ThreadsafeQueue<std::function<void(void)>>  tasks;
	std::thread backgroundTaskThread;
	void backgroundTaskThreadFunction();

	//our output, if any.
	std::shared_ptr<Device> device = nullptr;

	//the registered mixing matrices for this simulation.
	std::map<std::tuple<unsigned int, unsigned int>, float*> mixing_matrices;
	unsigned int largest_seen_mixing_matrix_input = 0;
	//used to apply mixing matrices when downmixing.
	float* mixing_matrix_workspace = nullptr;
	int tick_count = 0; //counts ticks.  This is part of node processing.
	int maintenance_start = 0; //also part of node processing. Used to stagger calls to doMaintenance on nodes so that we're not randomly spiking the tick length.
	int maintenance_rate = 5; //call on every 5th object.

	//support for the block callback.
	double block_callback_time = 0.0;
	LavBlockCallback block_callback = nullptr;
	void* block_callback_userdata =nullptr;
};

}