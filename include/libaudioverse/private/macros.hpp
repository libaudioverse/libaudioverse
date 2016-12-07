/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../libaudioverse.h"
#include "error.hpp" //needed by the standard catchblock macro, below.
#include "initialization.hpp"
#include <mutex>
#include <new>
#include <stdio.h>

/**These are the most common macros.
Also see logging.hpp, which has more.*/

//use __LINE__ for a quick unique variable.
#define LOCK(object) std::lock_guard<decltype((object))> lock_##__LINE__((object))

/*These macros define the beginning and end of a function intended to be exposed publicly.
Such functions should indicate errors by throwing ErrorException, not returning.  The first macro simply sets up a try block; the latter is a group of standard catch blocks that makes sure that
under no circumstances will an exception leave the public api and propagate into user code, ever.  This is necessary because we wish to bind to many languages, and very few actually support marshalling exceptions across the boundary.*/
#define PUB_BEGIN try {

#define PUB_END } catch(ErrorException &e) {\
recordError(e);\
return e.error; \
} catch(std::bad_alloc &e) {\
recordError(ErrorException(Lav_ERROR_MEMORY, std::string("Memory allocation error: ")+e.what(), __FILE__, __LINE__));\
return Lav_ERROR_MEMORY;\
} catch(std::exception &e) {\
recordError(ErrorException(Lav_ERROR_UNKNOWN, std::string("Standard library exception: ")+e.what(), __FILE__, __LINE__));\
return Lav_ERROR_UNKNOWN;\
} catch(...) {\
recordError(ErrorException(Lav_ERROR_UNKNOWN, "Unable to determine error. Thrown exception was not ErrorException.", __FILE__, __LINE__));\
return Lav_ERROR_UNKNOWN;\
}\
return Lav_ERROR_NONE;

//Either ERROR(code) or ERROR(code, message):
#define ERROR(...) throw ErrorException(__VA_ARGS__, __FILE__, __LINE__)

//Arrange to throw an error if we aren't initializerd.
//This is used on a few functions which we know have to be called before anything else.
//I.e. Since node creation needs a server, we need only check the server.
#define INITCHECK do {\
	if(isInitialized() == false) {\
		ERROR(Lav_ERROR_NOT_INITIALIZED, "You cannot use this function before initializing the library");\
	}\
} while(0)

#define STANDARD_PROPERTIES_BEGIN (-100)