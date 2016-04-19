/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../libaudioverse.h"
#include <string>

namespace libaudioverse_implementation {

/*This class serves one purpose: preventing us from catching and throwing integers.*/
class ErrorException {
	public:
	ErrorException() = default;
	//f is file, l is line, m is message, e is code.
	ErrorException(LavError e, std::string f, int l): error(e), file(f), line(l) {}
	ErrorException(LavError e, std::string m, std::string f, int l): error(e), line(l), message(m), file(f) {}
	LavError error = Lav_ERROR_NONE;
	std::string file, message;
	int line = 0;
};

void initializeErrorModule();
void shutdownErrorModule();
//Records this error for the current thread.
void recordError(ErrorException e);

}