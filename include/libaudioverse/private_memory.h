/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "uthash.h"
#include "private_threads.h"
#include "libaudioverse.h"
#ifdef _cplusplus
extern "c" {
#endif

typedef void (*LavFreeingFunction)(void*);

struct LavAllocatedPointer {
	void* pointer; //the pointer itself.
	LavFreeingFunction free; //what to call with the pointer as argument to get rid of it and all memory associated with it.
	UT_hash_handle hh; //magic internal field for uthash.
};
typedef struct LavAllocatedPointer LavAllocatedPointer;

struct LavMemoryManager {
	LavAllocatedPointer *managed_pointers;//through virtue of the magic of uthash, this is a hashtable.
	void* lock; //opaque lock handle from threading module, prevents threads from clobbering the hashtable.
};

typedef struct LavMemoryManager LavMemoryManager;

Lav_PUBLIC_FUNCTION LavMemoryManager* createMmanager();
Lav_PUBLIC_FUNCTION void freeMmanager(LavMemoryManager* manager);

//same as stdlib.
Lav_PUBLIC_FUNCTION void* mmanagerAlloc(LavMemoryManager* manager, size_t size);
Lav_PUBLIC_FUNCTION void* mmanagerCalloc(LavMemoryManager* manager, size_t elements, size_t size);
Lav_PUBLIC_FUNCTION void mmanagerFree(LavMemoryManager* manager, void* ptr);

//Declare ownership of a pointer, provided an appropriate function to free it.
//This is incredibly useful for file handles.  File handles are the whole reason for this function.
//it also makes internal operations easier.
Lav_PUBLIC_FUNCTION LavError mmanagerAssociatePointer(LavMemoryManager *manager, void* ptr, LavFreeingFunction freer);

#ifdef _cpulspuls
}
#endif