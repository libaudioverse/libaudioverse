/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/error.hpp>
#include <map>
#include <thread>

namespace libaudioverse_implementation {

std::map<std::thread::id, ErrorException> *thread_local_errors = nullptr;

void initializeErrorModule() {
	thread_local_errors = new std::map<std::thread::id, ErrorException>();
}

void shutdownErrorModule() {
	delete thread_local_errors;
}

void recordError(ErrorException e) {
	(*thread_local_errors)[std::this_thread::get_id()] = e;
}

Lav_PUBLIC_FUNCTION LavError Lav_errorGetMessage(const char** destination) {
	*destination = (*thread_local_errors)[std::this_thread::get_id()].message.c_str();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_errorGetFile(const char** destination) {
	*destination = (*thread_local_errors)[std::this_thread::get_id()].file.c_str();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_errorGetLine(int* destination) {
	*destination = (*thread_local_errors)[std::this_thread::get_id()].line;
	return Lav_ERROR_NONE;
}

}