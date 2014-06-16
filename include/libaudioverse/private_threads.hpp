/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"

typedef void (*LavThreadCapableFunction)(void* param);

LavError threadRun(LavThreadCapableFunction fn, void* param, void** destination);
LavError threadJoinAndFree(void* t);
LavError createMutex(void **destination);
LavError freeMutex(void *m);
LavError mutexLock(void *m);
LavError mutexUnlock(void *m);

/**The following functions are threadsafe ringbuffers*/
class LavCrossThreadRingBuffer {
	public:
	int read_position, write_position, element_size, length;
	void* lock;
	char* data;
	int last_op;
};

LavError createCrossThreadRingBuffer(int length, int elementSize, LavCrossThreadRingBuffer **destination);
int CTRBGetAvailableWrites(LavCrossThreadRingBuffer* buffer);
int CTRBGetAvailableReads(LavCrossThreadRingBuffer *buffer);
void CTRBGetItems(LavCrossThreadRingBuffer *buffer, int count, void* destination);
void CTRBWriteItems(LavCrossThreadRingBuffer *buffer, int count, void* data);

/**These are utilities that operate on the current thread.*/
void sleepFor(unsigned int milliseconds); //sleep.
void yield(); //yield this thread.

/**An atomic flag that can be either cleared or set.

The point of this is to allow for thraed killing and other such communications.  It is guaranteed to be lock-free on all architectures.*/
LavError createAFlag(void** destination);
int aFlagTestAndSet(void* flag);
void aFlagClear(void* flag);
void freeAFlag(void* flag);
