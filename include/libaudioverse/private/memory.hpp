/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <map>
#include <memory>
#include <string.h>
#include <mutex>
#include <atomic>
#include <functional>
#include "macros.hpp"
//contains some various memory-related bits and pieces, as well as the smart pointer marshalling.

namespace libaudioverse_implementation {

/**This is a standalone component that knows how to hold onto smart pointers, avoid accidentally duplicating entries, and cast the entries before giving them to us.

Asking for an entry that is not present gives a null pointer.

the reason that this is not a class is because there can only ever be one.  This serves the explicit purpose of allowing us to give pointers to and receive pointers from things outside Libaudioverse; therefore, it's a global and some helper functions.

This also implements Lav_free, which works the same for all pointers, and the object handle interfaces, most notably Lav_freeHandle.
*/

class ExternalObject;//declared in this header below the globals.
class Simulation;

extern std::map<void*, std::shared_ptr<void>> *external_ptrs;
extern std::map<int, std::shared_ptr<ExternalObject>> *external_handles;
extern std::map<int, std::weak_ptr<ExternalObject>> *weak_external_handles;
extern std::recursive_mutex *memory_lock;
//max handle we've used for an outgoing object. Ensures no duplication.
extern std::atomic<int> *max_handle;

class ExternalObject: public std::enable_shared_from_this<ExternalObject>  {
	public:
	ExternalObject(int type);
	virtual ~ExternalObject();
	int getType();
	bool is_external_object = false, is_first_external_access = false;
	int external_object_handle, type;
	//Have we been put in the dict yet?
	bool has_external_mapping = false;
	std::atomic<int> refcount;
};

template <class t>
std::shared_ptr<t> incomingPointer(void* ptr) {
	std::lock_guard<std::recursive_mutex> guard(*memory_lock);
	if(external_ptrs->count(ptr) == 0) return ERROR(Lav_ERROR_INVALID_POINTER, "Pointer did not originate from Libaudioverse or was deleted.");
	return std::static_pointer_cast<t, void>(external_ptrs->at(ptr));
}

template <class t>
t* outgoingPointer(std::shared_ptr<t> ptr) {
	std::lock_guard<std::recursive_mutex> guard(*memory_lock);
	t* out = ptr.get();
	if(out == nullptr) return nullptr;
	if(external_ptrs->count(out) == 1) return out;
	(*external_ptrs)[out] = ptr;
	return out;
}

template<class t>
int outgoingObject(std::shared_ptr<t> what) {
	//null is a special case for which we pass out 0.
	if(what == nullptr) return 0;
	if(what->has_external_mapping == false) {
		std::lock_guard<std::recursive_mutex> guard(*memory_lock);
		what->is_external_object =true;
		what->is_first_external_access = true;
		what->has_external_mapping = true;
		what->refcount.store(1);
		(*external_handles)[what->external_object_handle] = what;
		(*weak_external_handles)[what->external_object_handle] = what;
	}
	return what->external_object_handle;
}

template<class t>
std::shared_ptr<t> incomingObject(int handle, bool allowNull =false) {
	if(allowNull&& handle==0) return nullptr;
	std::lock_guard<std::recursive_mutex> guard(*memory_lock);
	if(external_handles->count(handle)) {
		auto res=std::dynamic_pointer_cast<t>(external_handles->at(handle));
		if(res == nullptr) ERROR(Lav_ERROR_TYPE_MISMATCH, "Incoming pointer did not match requested type.");
		return res;
	}
	else if(weak_external_handles->count(handle)) {
		auto weak_res = weak_external_handles->at(handle);
		auto strong_res = weak_res.lock();
		auto res = std::dynamic_pointer_cast<t>(strong_res);
		if(res ) return res;
		//The dynamic cast failing means we have a strong_res.
		//Not having a handle means no strong_res.
		else if(res == nullptr && strong_res != nullptr) ERROR(Lav_ERROR_TYPE_MISMATCH, "Incoming pointer did not match requested type.");
		else weak_external_handles->erase(handle);
	}
	ERROR(Lav_ERROR_INVALID_HANDLE, "Handle did not originate from Libaudioverse or was deleted.");
	//we can't get here, but some compilers probably complain anyway:
	return nullptr;
}

void initializeMemoryModule();
void shutdownMemoryModule();

/**This template uses memcpy to perform a safe type pun and avoid violating strict aliasing.*/
template<class t>
t safeConvertMemory(char* b) {
	t tmp;
	memcpy(&tmp, b, sizeof(t));
	return tmp;
}

/**Libaudioverse allocation routines.

These return and free pointers to zero-initialized memory aligned on the appropriate boundary for the enabled SIMD extensions, if any.  If no SIMD extensions are enabled, these gracefully fall back to normal calloc/free.
*/

template<class t>
t* allocArray(unsigned int size) {
	#if LIBAUDIOVERSE_MALLOC_ALIGNMENT == 1
	return (t*)calloc(size*sizeof(t), 1);
	#else
	//otherwise, we have this bit of fun.
	void* p1;
	void** p2;
	int offset = LIBAUDIOVERSE_MALLOC_ALIGNMENT-1+sizeof(void*);
	p1 = calloc(size*sizeof(t)+offset, 1);
	if(p1 == nullptr) return nullptr;
	p2 = (void**)(((intptr_t)(p1)+offset)&~(LIBAUDIOVERSE_MALLOC_ALIGNMENT-1));
	p2[-1]=p1;
	return (t*)p2;
	#endif
}

void freeArray(void* ptr);

//Check if a pointer is safe for SIMD.
bool isAligned(const void* ptr);

//custom deleter for smart pointer that guarantees thread safety.
std::function<void(ExternalObject*)> ObjectDeleter(std::shared_ptr<Simulation> simulation);


}