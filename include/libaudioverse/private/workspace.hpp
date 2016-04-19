/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "memory.hpp"
#include <cstddef>

namespace libaudioverse_implementation {

/**A resizable workspace.

The intent of this class is to be used with thread_local, to make some objects that need scratch space use less ram.
I.e.:
thread_local Workspace<flaot> ws;
And then, inside a function (and only inside a function):
float*ptr = ws.get(aSize);

The workspace grows to fit and is destroyed with the thread. Do not use this for large chunks of ram.
*/

template<typename T>
class Workspace {
	public:
	~Workspace() {
		if(ptr) freeArray(ptr);
	}
	
	T* get(std::size_t size) {
		if(size > this->size) {
			if(ptr) freeArray(ptr);
			ptr = allocArray<T>(size);
			this->size = size;
		}
		return ptr;
	}
	
	private:
	T* ptr = nullptr;
	std::size_t size = 0;
};

}