/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/logging.hpp>
#include <logger_singleton/logger_singleton.hpp>
#include <logger_singleton/logger_singleton.hpp>
#include <atomic>
#include <mutex>
#include <string>
#include <boost/thread/once.hpp>

namespace libaudioverse_implementation {

boost::once_flag logging_init_flag;
bool initialized_logging = false;
std::atomic<LavLoggingCallback> saved_logging_callback;
std::shared_ptr<logger_singleton::Logger> *logger = nullptr;

void implicitInitLogging() {
	boost::call_once(logging_init_flag, [] () {
		logger = new std::shared_ptr<logger_singleton::Logger>();
		*logger = logger_singleton::createLogger();
		initialized_logging = true;
		saved_logging_callback.store(nullptr);
	});
}

void shutdownLogging() {
	if(initialized_logging) {
		delete logger;
	}
}

logger_singleton::LoggingLevel intToLoggingLevel(int level) {
	switch(level) {
		case Lav_LOGGING_LEVEL_OFF: return logger_singleton::LoggingLevel::OFF;
		case Lav_LOGGING_LEVEL_CRITICAL: return logger_singleton::LoggingLevel::CRITICAL;
		case Lav_LOGGING_LEVEL_INFO: return logger_singleton::LoggingLevel::INFO;
		case Lav_LOGGING_LEVEL_DEBUG: return logger_singleton::LoggingLevel::DEBUG;
	}
	return logger_singleton::LoggingLevel::CRITICAL;
}

int loggingLevelToInt(logger_singleton::LoggingLevel level) {
	switch(level) {
		case logger_singleton::LoggingLevel::OFF: return Lav_LOGGING_LEVEL_OFF;
		case logger_singleton::LoggingLevel::CRITICAL: return Lav_LOGGING_LEVEL_CRITICAL;
		case logger_singleton::LoggingLevel::INFO: return Lav_LOGGING_LEVEL_INFO;
		case logger_singleton::LoggingLevel::DEBUG: return Lav_LOGGING_LEVEL_DEBUG;
	}
	return Lav_LOGGING_LEVEL_OFF;
}

void logCallbackTranslator(logger_singleton::LogMessage &msg, LavLoggingCallback cb) {
	std::string outgoing = msg.topic+": "+msg.message;
	cb(loggingLevelToInt(msg.level), outgoing.c_str());
}

std::shared_ptr<logger_singleton::Logger> getLogger() {
	implicitInitLogging();
	return *logger;
}

//begin public api.
Lav_PUBLIC_FUNCTION LavError Lav_setLoggingCallback(LavLoggingCallback cb) {
	PUB_BEGIN
	implicitInitLogging();
	getLogger()->setLoggingCallback([=] (logger_singleton::LogMessage &msg) {logCallbackTranslator(msg, cb);});
	saved_logging_callback.store(cb);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_getLoggingCallback(LavLoggingCallback* destination) {
	PUB_BEGIN
	implicitInitLogging();
	*destination = saved_logging_callback.load();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_setLoggingLevel(int level) {
	PUB_BEGIN
	implicitInitLogging();
	getLogger()->setLoggingLevel(intToLoggingLevel(level));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_getLoggingLevel(int* destination) {
	PUB_BEGIN
	implicitInitLogging();
	*destination = loggingLevelToInt(getLogger()->getLoggingLevel());
	PUB_END
}

}