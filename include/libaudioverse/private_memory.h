#pragma once
#include <uthash.h>
#include "private_threads.h"

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

LavMemoryManager* createMmanager();
void FreeMmanager(LavMemoryManager* manager);
//same as stdlib.
void* mmanagerAlloc(LavMemoryManager* manager, size_t size);
void* mmanagerCalloc(LavMemoryManager* manager, size_t elements, size_t size);
void mmanagerFree(LavMemoryManager* manager, void* ptr);

//Declare ownership of a pointer, provided an appropriate function to free it.
//This is incredibly useful for file handles.  File handles are the whole reason for this function.
//it also makes internal operations easier.
void managerAssociatePointer(void* ptr, LavFreeingFunction free);
