/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LavThreadCapableFunction)(void* param);

Lav_PUBLIC_FUNCTION LavError threadRun(LavThreadCapableFunction fn, void* param, void** destination);
Lav_PUBLIC_FUNCTION LavError threadJoinAndFree(void* t);
Lav_PUBLIC_FUNCTION LavError createMutex(void **destination);
Lav_PUBLIC_FUNCTION LavError freeMutex(void *m);
Lav_PUBLIC_FUNCTION LavError mutexLock(void *m);
Lav_PUBLIC_FUNCTION LavError mutexUnlock(void *m);

/**The following functions are threadsafe ringbuffers*/
typedef struct Lav_CrossThreadRingBuffer_s LavCrossThreadRingBuffer;
Lav_PUBLIC_FUNCTION LavError createCrossThreadRingBuffer(int length, int elementSize, LavCrossThreadRingBuffer **destination);
Lav_PUBLIC_FUNCTION int CTRBGetAvailableWrites(LavCrossThreadRingBuffer* buffer);
Lav_PUBLIC_FUNCTION int CTRBGetAvailableReads(LavCrossThreadRingBuffer *buffer);
Lav_PUBLIC_FUNCTION void CTRBGetItems(LavCrossThreadRingBuffer *buffer, int count, void* destination);
Lav_PUBLIC_FUNCTION void CTRBWriteItems(LavCrossThreadRingBuffer *buffer, int count, void* data);

/**These are utilities that operate on the current thread.*/
Lav_PUBLIC_FUNCTION void sleepFor(unsigned int milliseconds); //sleep.
Lav_PUBLIC_FUNCTION void yield(); //yield this thread.

/**An atomic flag that can be either cleared or set.

The point of this is to allow for thraed killing and other such communications.  It is guaranteed to be lock-free on all architectures.*/
Lav_PUBLIC_FUNCTION LavError createAFlag(void** destination);
Lav_PUBLIC_FUNCTION int aFlagTestAndSet(void* flag);
Lav_PUBLIC_FUNCTION void aFlagClear(void* flag);
Lav_PUBLIC_FUNCTION void freeAFlag(void* flag);

#ifdef __cplusplus
}
#endif
