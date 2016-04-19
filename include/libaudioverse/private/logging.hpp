/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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