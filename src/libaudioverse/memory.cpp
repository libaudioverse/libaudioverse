/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/error.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <atomic>
#include <functional>

namespace libaudioverse_implementation {

std::map<void*, std::shared_ptr<void>> *external_ptrs = nullptr;
std::recursive_mutex *memory_lock = nullptr;
std::map<int, std::shared_ptr<ExternalObject>> *external_handles = nullptr;
std::atomic<int> *max_handle = nullptr;
LavHandleDestroyedCallback handle_destroyed_callback = nullptr;

void initializeMemoryModule() {
	memory_lock=new std::recursive_mutex();
	max_handle = new std::atomic<int>();
	max_handle->store(1);
	external_ptrs= new std::map<void*, std::shared_ptr<void>>();
	external_handles=new std::map<int, std::shared_ptr<ExternalObject>>();
}

void shutdownMemoryModule() {
	delete external_handles;
	delete external_ptrs;
	delete max_handle;
	delete memory_lock;
}

ExternalObject::ExternalObject(int type) {
	externalObjectHandle = max_handle->fetch_add(1);
	this->type=type;
	refcount.store(0);
}

ExternalObject::~ExternalObject() {
	//We can't call the handleDestroyedCallback here, if we do we're inside a lock.
	//This is here because the ExternalObject constructor is virtual and this used to contain code (and might again).
}

int ExternalObject::getType() {
	return type;
}

void freeArray(void* ptr) {
	#if LIBAUDIOVERSE_MALLOC_ALIGNMENT == 1
	free(ptr);
	#else
	void* p = ((void**)ptr)[-1];
	free(p);
	#endif
}

bool isAligned(const void* ptr) {
	#if LIBAUDIOVERSE_MALLOC_ALIGNMENT==1
	return true;
	#else
	intptr_t mask = LIBAUDIOVERSE_MALLOC_ALIGNMENT-1;
	intptr_t  i = (intptr_t)ptr;
	//all the low bits have to be clear.
	return (i & mask) == 0;
	#endif
}
std::function<void(ExternalObject*)> ObjectDeleter(std::shared_ptr<Simulation> simulation) {
	return [=](ExternalObject* obj) {
		//We have to make sure to call the callback outside the lock.
		//To that end, we gather information as follows, and then queue it.
		bool isExternal;
		int handle;
		LOCK(*simulation);
		isExternal = obj->isExternalObject;
		handle = obj->externalObjectHandle;
		//WARNING: this line can call this deleter recursively, if obj contains the final shared pointer to another ExternalObject.
		delete obj;
		//Because of the recursion, shell out to the simulation's task thread.
		if(isExternal && handle_destroyed_callback) simulation->enqueueTask([handle] () {handle_destroyed_callback(handle);});
	};
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_free(void* ptr) {
	PUB_BEGIN
	auto guard = std::lock_guard<std::recursive_mutex>(*memory_lock);
	if(external_ptrs->count(ptr)) external_ptrs->erase(ptr);
	else ERROR(Lav_ERROR_INVALID_HANDLE);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_handleIncRef(LavHandle handle) {
	PUB_BEGIN
	auto e = incomingObject<ExternalObject>(handle);
	e->refcount.fetch_add(1);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_handleDecRef(LavHandle handle) {
	PUB_BEGIN
	auto e = incomingObject<ExternalObject>(handle);
	auto rc = e->refcount.fetch_add(-1);
	rc-=1;
	if(rc == 0) {
		auto guard=std::lock_guard<std::recursive_mutex>(*memory_lock);
		external_handles->erase(e->externalObjectHandle);
	}
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_handleGetAndClearFirstAccess(LavHandle handle, int* destination) {
	PUB_BEGIN
	auto e = incomingObject<ExternalObject>(handle);
	*destination = e->isFirstExternalAccess;
	e->isFirstExternalAccess = false;
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_handleGetRefCount(LavHandle handle, int* destination) {
	PUB_BEGIN
	auto e =incomingObject<ExternalObject>(handle);
	*destination =e->refcount.load();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_handleGetType(LavHandle handle, int* destination) {
	PUB_BEGIN
	auto e = incomingObject<ExternalObject>(handle);
	*destination = e->getType();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_setHandleDestroyedCallback(LavHandleDestroyedCallback cb) {
	PUB_BEGIN
	if(memory_lock) memory_lock->lock();
	handle_destroyed_callback=cb;
	if(memory_lock) memory_lock->unlock();
	PUB_END
}

}