/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "memory.hpp"
#include <memory>
#include <atomic>


namespace libaudioverse_implementation {
class Simulation;

class Buffer: public ExternalObject {
	public:
	Buffer(std::shared_ptr<Simulation> simulation);
	~Buffer();
	std::shared_ptr<Simulation> getSimulation();
	int getLength();
	double getDuration();
	int getChannels();
	//This can be used outside the lock; the only thing it does is read simulation's sr value which can never change by definition.
	void loadFromArray(int sr, int channels, int frames, float* inputData);
	//The following two functions do not check if the requested frame is past the end for efficiency.
	//It is possible the compiler would optimize this, but running  in debug mode is already really painful and the trade-off here is worth it.
	//a single sample without mixing:
	float getSample(int frame, int channel);
	//Get a pointer to part of the buffer, so that we can memcpy and stuff.
	float* getPointer(int frame, int channel);
	//meet lockable concept:
	void lock();
	void unlock();

	//Normalize the buffer: divide by the sample furthest from zero.
	//This can't be undone.
	void normalize();
	
	//Lock and unlock the user's ability to change the buffer's contents.
	//These three functions are technically threadsafe, as they all use an atomic variable.
	void incrementUseCount();
	void decrementUseCount();
	void throwIfInUse();
	private:
	int channels = 0;
	int frames = 0;
	int sr = 0;
	float* data = nullptr;
	std::shared_ptr<Simulation> simulation;
	std::atomic<int> use_count{0};
};

std::shared_ptr<Buffer> createBuffer(std::shared_ptr<Simulation>simulation);

}