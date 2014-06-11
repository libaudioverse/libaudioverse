/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_threads.h"
#include "private_memory.h"
//this is only macros; consequently, no ifdef __cplusplus.

/**Private macro definitions.*/
//pi.
#define PI (3.14159)

/**The following three macros abstract returning error codes, and make the cleanup logic for locks manageable.
They exist because goto is a bad thing for clarity, and because they can.*/

#define STANDARD_PREAMBLE LavError return_value;\
int did_already_lock = 0;\
void*localMemoryManager = createMmanager();\
void* globalMemoryManager = mmanagerGetGlobalMemoryManager();\
ERROR_IF_TRUE(localMemoryManager == NULL || globalMemoryManager == NULL, Lav_ERROR_MEMORY);

#define SAFERETURN(value) do {\
return_value = value;\
goto do_return_and_cleanup;\
} while(0)

#define BEGIN_CLEANUP_BLOCK do_return_and_cleanup:

#define DO_ACTUAL_RETURN return return_value

#define STANDARD_CLEANUP_BLOCK BEGIN_CLEANUP_BLOCK \
freeMmanager(localMemoryManager);\
DO_ACTUAL_RETURN

#define LOCK(lock_expression) mutexLock((lock_expression));\
mmanagerAssociatePointer(localMemoryManager, (lock_expression), mutexUnlock);

#define ERROR_IF_TRUE(expression, error) do {\
if(expression) SAFERETURN(error);\
} while(0)

#define CHECK_NOT_NULL(ptr) ERROR_IF_TRUE(ptr == NULL, Lav_ERROR_NULL_POINTER)

//gcc and clang both have _static_assert, but visual studio names it differently.
#ifdef _MSC_VER
#define _Static_assert(a, b) static_assert(a, b)
#endif
