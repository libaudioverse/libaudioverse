/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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
//The handles we keep alive because the external world has a copy.
std::map<int, std::shared_ptr<ExternalObject>> *external_handles = nullptr;
//Sometimes, the external world has handed in all its references.  But we need to be able to look it up anyway, in case we haven't deleted it.
//This dict holds a weak reference to every handle passed to the external world until the end of the program.
std::map<int, std::weak_ptr<ExternalObject>> *weak_external_handles = nullptr;
std::atomic<int> *max_handle = nullptr;
LavHandleDestroyedCallback handle_destroyed_callback = nullptr;
bool memory_initialized = false;

void initializeMemoryModule() {
	memory_lock=new std::recursive_mutex();
	max_handle = new std::atomic<int>();
	max_handle->store(1);
	external_ptrs= new std::map<void*, std::shared_ptr<void>>();
	external_handles=new std::map<int, std::shared_ptr<ExternalObject>>();
	weak_external_handles = new std::map<int, std::weak_ptr<ExternalObject>>();
	memory_initialized = true;
}

void shutdownMemoryModule() {
	std::lock_guard<std::recursive_mutex> l(*memory_lock);
	//We're about to shut down, but sometimes there are cycles.
	//The most notable case of this is nodes connected to the simulation: the simulation holds them and they hold the simulation.
	//In order to help prevent bugs, we therefore isolate all nodes that we can reach.
	//In addition, simulations hold devices which may be in the middle of processing.
	//In this case, the simulation needs to be isolated here--if we don't, we can abandon its pointer while it's still running.
	//This additionally results in a running thread that we never join.
	for(auto &i: *weak_external_handles) {
		auto s = i.second.lock();
		auto n = std::dynamic_pointer_cast<Node>(s);
		auto sim = std::dynamic_pointer_cast<Simulation>(s);
		if(n) n->isolate();
		else if(sim) {
			sim->clearOutputDevice();
		}
	}
	delete external_handles;
	external_handles = nullptr;
	delete weak_external_handles;
	weak_external_handles = nullptr;
	delete external_ptrs;
	external_ptrs = nullptr;
	delete max_handle;
	max_handle = nullptr;
	//We intensionally leak the memory lock.
	//This has to stay around so that the public API can be made safe after library shutdown.
	//User code won't call us, but garbage collected languages might.
	memory_initialized = false;
}

ExternalObject::ExternalObject(int type) {
	external_object_handle = max_handle->fetch_add(1);
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
	return [=](ExternalObject* obj) mutable {
		//We have to make sure to call the callback outside the lock.
		//To that end, we gather information as follows, and then queue it.
		bool isExternal;
		int handle;
		LOCK(*simulation);
		isExternal = obj->is_external_object;
		handle = obj->external_object_handle;
		//WARNING: this line can call this deleter recursively, if obj contains the final shared pointer to another ExternalObject.
		delete obj;
		//Because of the recursion, shell out to the simulation's task thread.
		if(isExternal && handle_destroyed_callback) simulation->enqueueTask([handle] () {handle_destroyed_callback(handle);});
		//The simulation holds weak_ptrs to nodes.
		//weak_ptrs hold references to this deleter.
		//Therefore there is a cycle.
		simulation = nullptr;
	};
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_free(void* ptr) {
	PUB_BEGIN
	std::lock_guard<std::recursive_mutex> guard(*memory_lock);
	if(memory_initialized == false) return Lav_ERROR_NONE; //If we're shut down, everything is already freed.
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
	std::unique_lock<std::recursive_mutex> l(*memory_lock);
	if(memory_initialized == false) return Lav_ERROR_NONE;
	auto e = incomingObject<ExternalObject>(handle);
	auto rc = e->refcount.fetch_add(-1);
	rc-=1;
	if(rc == 0) {
		external_handles->erase(e->external_object_handle);
		//We need to be readded to the dict if we're passed out again.
		e->has_external_mapping = false;
	}
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_handleGetAndClearFirstAccess(LavHandle handle, int* destination) {
	PUB_BEGIN
	auto e = incomingObject<ExternalObject>(handle);
	*destination = e->is_first_external_access;
	e->is_first_external_access = false;
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