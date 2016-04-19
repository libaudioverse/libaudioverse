/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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