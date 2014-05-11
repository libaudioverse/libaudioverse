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


/**The following three macros abstract returning error codes, and make the cleanup logic for locks manageable.
They exist because goto is a bad thing for clarity, and because they can.*/

#define WILL_RETURN(type) type return_value; int did_already_lock = 0

#define RETURN(value) do {\
return_value = value;\
goto do_return_and_cleanup;\
} while(0)

#define BEGIN_CLEANUP_BLOCK do_return_and_cleanup:

#define DO_ACTUAL_RETURN return return_value

#define STANDARD_CLEANUP_BLOCK(mutex) BEGIN_CLEANUP_BLOCK \
if(did_already_lock) mutexUnlock((mutex));\
DO_ACTUAL_RETURN

#define LOCK(lock_expression) mutexLock((lock_expression));\
did_already_lock = 1;

#ifdef __cplusplus
}
#endif
