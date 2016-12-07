/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/error.hpp>
#include <map>
#include <thread>

namespace libaudioverse_implementation {

std::map<std::thread::id, ErrorException> thread_local_errors;

void recordError(ErrorException e) {
	thread_local_errors[std::this_thread::get_id()] = e;
}

Lav_PUBLIC_FUNCTION LavError Lav_errorGetMessage(const char** destination) {
	*destination = thread_local_errors[std::this_thread::get_id()].message.c_str();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_errorGetFile(const char** destination) {
	*destination = thread_local_errors[std::this_thread::get_id()].file.c_str();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_errorGetLine(int* destination) {
	*destination = thread_local_errors[std::this_thread::get_id()].line;
	return Lav_ERROR_NONE;
}

}