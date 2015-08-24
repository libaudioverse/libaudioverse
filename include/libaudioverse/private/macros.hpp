/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <mutex>
#include <new>
#include "../libaudioverse.h"
#include "error.hpp" //needed by the standard catchblock macro, below.

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

#define STANDARD_PROPERTIES_BEGIN (-100)