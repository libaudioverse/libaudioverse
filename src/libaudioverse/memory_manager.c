/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>
#include <utarray.h>
#include <uthash.h>

/**This file uses uthash to implement local and global memory contexts.*/

LavMemoryManager* createMmanager() {
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

void freeMmanager(LavMemoryManager* manager) {
	if(manager == NULL) {
		return;
	}
	mutexLock(manager->lock);
	//we can't fall back to our custom free: if we do, we're modifying the has while we iterate, and that is not a safe thing.
	LavAllocatedPointer *pointer, *tmp; //needed for iteration by uthash.
	HASH_ITER(hh, manager->managed_pointers, pointer, tmp) {
		pointer->free(pointer->pointer);
	}
	mutexUnlock(manager->lock);
	freeMutex(manager->lock);
	free(manager);
}

LavError mmanagerAssociatePointer(LavMemoryManager* manager, void* ptr, LavFreeingFunction freer) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(manager);
	CHECK_NOT_NULL(ptr);
	CHECK_NOT_NULL(freer);
	LOCK(manager->lock);
	LavAllocatedPointer* assoc = calloc(1, sizeof(LavAllocatedPointer));
	ERROR_IF_TRUE(assoc == NULL, Lav_ERROR_MEMORY);
	assoc->pointer = ptr;
	assoc->free = freer;
	HASH_ADD_PTR(manager->managed_pointers, pointer, assoc);
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(manager->lock);
}

void* mmanagerAlloc(LavMemoryManager* manager, size_t size) {
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

void* mmanagerCalloc(LavMemoryManager* manager, size_t elements, size_t size) {
	void* ptr = mmanagerAlloc(manager, elements*size);
	if(ptr == NULL) {
		return NULL;
	}
	memset(ptr, 0, elements*size);
	return ptr;
}

void mmanagerFree(LavMemoryManager* manager, void* ptr) {
	LavAllocatedPointer *entry = NULL;
	mutexLock(manager->lock);
	HASH_FIND_PTR(manager->managed_pointers, ptr, entry);
	if(entry != NULL) {
		entry->free(entry->pointer);
	}
	mutexUnlock(manager->lock);
}
