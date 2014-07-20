/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <lambdatask/threadsafe_queue.hpp>
#include <functional> //we have to use an std::function for the preprocessing hook.  There's no good way around it because worlds need to use capturing lambdas.
#include <set>
#include <vector>
#include <mutex>
#include <memory>
#include <thread>
#include "libaudioverse.h"

class LavObject;

/*When thrown on the background thread, terminates it.*/
class LavThreadTerminationException {
};

class LavDevice {
	public:
	LavDevice(unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixahead);
	virtual ~LavDevice();
	virtual LavError getBlock(float* out);
	virtual unsigned int getBlockSize() { return block_size;}
	virtual LavError start();
	virtual LavError stop();
	virtual LavError associateObject(std::shared_ptr<LavObject> obj);
	virtual std::shared_ptr<LavObject> getOutputObject();
	virtual LavError setOutputObject(std::shared_ptr<LavObject> obj);
	virtual float getSr() { return sr;}
	virtual int getChannels() {return channels;}

	//these make us meet the basic lockable concept.
	void lock() {mutex.lock();}
	void unlock() {mutex.unlock();}
	void enqueueTask(std::function<void(void)>);

	protected:
	//visit all objects in the order they need to be visited if we were processing the graph.
	virtual void visitAllObjectsInProcessOrder(std::function<void(std::shared_ptr<LavObject>)> visitor);
	//visit all objects in the order they must be visited to prepare for and process obj for a block of audio.  This is not the same as all parents: this call respects suspended.
	virtual void visitForProcessing(std::shared_ptr<LavObject> obj, std::function<void(std::shared_ptr<LavObject>)> visitor);
	std::function<void(void)> preprocessing_hook;
	//this is a reusable vector that we allow to grow, but clear every tick.  Holds nodes in the order we need to process them.
	std::vector<std::shared_ptr<LavObject>> process_order;
	unsigned int block_size = 0, channels = 0, mixahead = 0, is_started = 0;
	float sr = 0.0f;
	//if objects die, they automatically need to be removed.  We can do said removal on next process.
	std::set<std::weak_ptr<LavObject>, std::owner_less<std::weak_ptr<LavObject>>> objects, always_process;
	std::shared_ptr<LavObject> output_object = nullptr;
	std::recursive_mutex mutex;

	lambdatask::ThreadsafeQueue<std::function<void(void)>>  tasks;
	std::thread backgroundTaskThread;
	void backgroundTaskThreadFunction();
};

//initialize the audio backend.
void initializeAudioBackend();

std::shared_ptr<LavDevice> createPortaudioDevice(unsigned int sr, unsigned int channels, unsigned int bufferSize, unsigned int mixahead);
