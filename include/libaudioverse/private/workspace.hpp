/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "memory.hpp"
#include <cstddef>

namespace libaudioverse_implementation {

/**A resizable workspace.

The intent of this class is to be used with thread_local, to make some objects that need scratch space use less ram.
I.e.:
thread_local Workspace<float> ws;
And then, inside a function (and only inside a function):
float*ptr = ws.get(aSize);

The workspace grows to fit and is destroyed with the thread. Do not use this for large chunks of ram.

By default, the workspace zeros the buffer before returning it.
*/

template<typename T>
class Workspace {
	public:
	~Workspace() {
		if(ptr) freeArray(ptr);
	}
	
	T* get(std::size_t size, bool zeroFirst = true) {
		if(size > this->size) {
			if(ptr) freeArray(ptr);
			ptr = allocArray<T>(size);
			this->size = size;
		}
		if(zeroFirst) memset(ptr, 0, sizeof(T)*size);
		return ptr;
	}
	
	private:
	T* ptr = nullptr;
	std::size_t size = 0;
};

}