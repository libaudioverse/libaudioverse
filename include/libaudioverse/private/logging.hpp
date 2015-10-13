/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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