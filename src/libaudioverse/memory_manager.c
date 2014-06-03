/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>
#include <libaudioverse/uthash.h>

/*This file uses uthash to implement local and global memory contexts.*/

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

Lav_PUBLIC_FUNCTION void* createMmanager() {
	LavMemoryManager *manager = NULL;
	manager = calloc(1, sizeof(LavMemoryManager));
	if(manager == NULL) {
		return NULL;
	}
	LavError err = createMutex(&manager->lock);
	if(err != Lav_ERROR_NONE) {
		return NULL;
	}
	return manager;
}

Lav_PUBLIC_FUNCTION void freeMmanager(void* manager) {
	if(manager == NULL) {
		return;
	}
	LavMemoryManager* m = manager;
	mutexLock(m->lock);
	//we can't fall back to our custom free: if we do, we're modifying the has while we iterate, and that is not a safe thing.
	LavAllocatedPointer *pointer, *tmp; //needed for iteration by uthash.
	HASH_ITER(hh, m->managed_pointers, pointer, tmp) {
		HASH_DEL(m->managed_pointers, pointer);
		pointer->free(pointer->pointer);
		free(pointer);
	}
	mutexUnlock(m->lock);
	freeMutex(m->lock);
	free(m);
}

Lav_PUBLIC_FUNCTION LavError mmanagerAssociatePointer(void* manager, void* ptr, LavFreeingFunction freer) {
	if(manager == NULL || freer == NULL || ptr == NULL) {
		return Lav_ERROR_NULL_POINTER;
	}
	LavMemoryManager* m = manager;
	mutexLock(m->lock);
	LavAllocatedPointer* assoc = calloc(1, sizeof(LavAllocatedPointer));
	if(assoc == NULL) {
		mutexUnlock(m->lock);
		return Lav_ERROR_NONE;
	}
	assoc->pointer = ptr;
	assoc->free = freer;
	HASH_ADD_PTR(m->managed_pointers, pointer, assoc);
	mutexUnlock(m->lock);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION void* mmanagerMalloc(void* manager, unsigned int size) {
	if(manager == NULL) {
		return NULL;
	}
	void* ptr = malloc(size);
	if(ptr == NULL) {
		return NULL;
	}
	if(mmanagerAssociatePointer(manager, ptr, free) != Lav_ERROR_NONE) {
		return NULL;
	};
	return ptr;
}

Lav_PUBLIC_FUNCTION void* mmanagerCalloc(void* manager, unsigned int elements, unsigned int size) {
	void* ptr = mmanagerMalloc(manager, elements*size);
	if(ptr == NULL) {
		return NULL;
	}
	memset(ptr, 0, elements*size);
	return ptr;
}

Lav_PUBLIC_FUNCTION void mmanagerFree(void* manager, void* ptr) {
	LavMemoryManager* m = manager;
	LavAllocatedPointer *entry = NULL;
	mutexLock(m->lock);
	HASH_FIND_PTR(m->managed_pointers, ptr, entry);
	if(entry != NULL) {
		HASH_DEL(m->managed_pointers, entry);
		entry->free(entry->pointer);
		free(entry);
	}
	mutexUnlock(m->lock);
}
