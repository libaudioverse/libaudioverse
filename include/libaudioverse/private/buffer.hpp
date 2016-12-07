/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "memory.hpp"
#include <memory>
#include <atomic>


namespace libaudioverse_implementation {
class Server;

class Buffer: public ExternalObject {
	public:
	Buffer(std::shared_ptr<Server> server);
	~Buffer();
	std::shared_ptr<Server> getServer();
	int getLength();
	double getDuration();
	int getChannels();
	//This can be used outside the lock; the only thing it does is read server's sr value which can never change by definition.
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
	std::shared_ptr<Server> server;
	std::atomic<int> use_count{0};
};

std::shared_ptr<Buffer> createBuffer(std::shared_ptr<Server>server);

}