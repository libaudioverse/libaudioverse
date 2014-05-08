#pragma once
#include "libaudioverse.h"

/**Private macro definitions.*/

#define ERROR_IF_TRUE(expression, error) do {\
if(expression) RETURN(error);\
} while(0)

#define CHECK_NOT_NULL(ptr) ERROR_IF_TRUE(ptr == NULL, Lav_ERROR_NULL_POINTER)

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
if(did_already_lock) SDL_UnlockMutex((mutex));\
DO_ACTUAL_RETURN

#define LOCK(lock_expression) SDL_LockMutex((lock_expression));\
did_already_lock = 1;
