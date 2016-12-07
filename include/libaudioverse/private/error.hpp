/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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

//Records this error for the current thread.
void recordError(ErrorException e);

}