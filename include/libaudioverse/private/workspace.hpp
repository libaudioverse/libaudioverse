/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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