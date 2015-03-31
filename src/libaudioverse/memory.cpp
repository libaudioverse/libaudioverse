/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/errors.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <atomic>

std::map<void*, std::shared_ptr<void>> external_ptrs;
std::mutex memory_lock;
std::map<int, std::shared_ptr<LavExternalObject>> external_handles;
std::atomic<int> max_handle;

void initializeMemoryModule() {
	max_handle.store(0);
}

Lav_PUBLIC_FUNCTION LavError Lav_free(void* ptr) {
	PUB_BEGIN
	auto guard = std::lock_guard<std::mutex>(memory_lock);
	if(external_ptrs.count(ptr)) external_ptrs.erase(ptr);
	else throw LavErrorException(Lav_ERROR_INVALID_HANDLE);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_freeHandle(LavHandle h) {
	PUB_BEGIN
	auto guard=std::lock_guard<std::mutex>(memory_lock);
	if(external_handles.count(h)) external_handles.erase(h);
	else throw LavErrorException(Lav_ERROR_INVALID_HANDLE);
	PUB_END
}


float* LavAllocFloatArray(unsigned int size) {
	#if LIBAUDIOVERSE_MALLOC_ALIGNMENT == 1
	return (float*)calloc(size*sizeof(float), 1);
	#else
	//otherwise, we have this bit of fun.
	void* p1;
	void** p2;
	int offset = LIBAUDIOVERSE_MALLOC_ALIGNMENT-1+sizeof(void*);
	p1 = calloc(size*sizeof(float)+offset, 1);
	if(p1 == nullptr) return nullptr;
	p2 = (void**)(((intptr_t)(p1)+offset)&~(LIBAUDIOVERSE_MALLOC_ALIGNMENT-1));
	p2[-1]=p1;
	return (float*)p2;
	#endif
}

void LavFreeFloatArray(float* ptr) {
	#if LIBAUDIOVERSE_MALLOC_ALIGNMENT == 1
	free(ptr);
	#else
	void* p = ((void**)ptr)[-1];
	free(p);
	#endif
}

void LavNodeDeleter(LavNode*node) {
	auto sim = node->getSimulation();
	LOCK(*sim);
	delete node;
}
