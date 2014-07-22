/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_macros.hpp>
#include <map>
#include <memory>
#include <mutex>

std::map<void*, std::shared_ptr<void>> *external_ptrs;
std::mutex *memory_lock;
void initializeMemoryModule() {
	external_ptrs = new std::map<void*, std::shared_ptr<void>>();
	memory_lock = new std::mutex();
}

Lav_PUBLIC_FUNCTION LavError Lav_free(void* ptr) {
	PUB_BEGIN
	auto guard = std::lock_guard<std::mutex>(*memory_lock);
	if(external_ptrs->count(ptr)) external_ptrs->erase(ptr);
	PUB_END
}
