#pragma once
#include <uthash.h>
#include "private_threads.h"

typedef void (*LavFreeingFunction)(void*);

struct LavAllocatedPointer {
	void* id; //the pointer itself, named specially for uthash.
	LavFreeingFunction free; //what to call with the pointer as argument to get rid of it and all memory associated with it.
	UT_hash_handle hh; //magic internal field for uthash.
};
typedef struct LavAllocatedPointer LavAllocatedPointer;

struct LavMemoryManager {
	LavAllocatedPointer *managed_pointers;//through virtue of the magic of uthash, this is a hashtable.
	void* lock; //opaque lock handle from threading module, prevents threads from clobbering the hashtable.
};

typedef struct LavMemoryManager LavMemoryManager;

LavMemoryManager* createMemoryManager();
//same as stdlib.
void* managerAlloc(LavMemoryManager* manager, size_t size);
void* managerCalloc(LavMemoryManager* manager, size_t elements, size_t size);
void managerFree(LavMemoryManager* manager, void* ptr);
