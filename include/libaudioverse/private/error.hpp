/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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