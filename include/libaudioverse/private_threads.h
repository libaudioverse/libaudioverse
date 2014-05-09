#pragma once

#ifdef __cplusplus
extern "C" {
#endif
typedef void* LavMutex;

LavError createMutex(LavMutex **destination);
LavError freeMutex(LavMutex *m);
LavError lockMutex(LavMutex *m);
LavError unlockMutex(LavMutex *m);

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
if(did_already_lock) unlockMutex((mutex));\
DO_ACTUAL_RETURN

#define LOCK(lock_expression) lockMutex((lock_expression));\
did_already_lock = 1;

#ifdef __cplusplus
}
#endif
