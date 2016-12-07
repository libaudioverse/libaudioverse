/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include <logger_singleton/logger_singleton.hpp>
#include <string>

namespace libaudioverse_implementation {

/**Initialization of logging is implicit via once_flag.  Shutdown is not.
We hide the implicit initialization behind this function to avoid dragging in more functions here.*/
void implicitInitLogging();
void shutdownLogging();

std::shared_ptr<logger_singleton::Logger> getLogger();

template<typename... ArgsT>
void logCritical(std::string format, ArgsT... args) {
	getLogger()->logCritical("libaudioverse", format, args...);
}

template<typename... ArgsT>
void logInfo(std::string format, ArgsT... args) {
	getLogger()->logInfo("libaudioverse", format, args...);
}

template<typename... ArgsT>
void logDebug(std::string format, ArgsT... args) {
	getLogger()->logDebug("libaudioverse", format, args...);
}

}