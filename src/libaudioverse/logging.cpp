/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/logging.hpp>
#include <logger_singleton.hpp>
#include <atomic>
#include <mutex>
#include <string>

namespace libaudioverse_implementation {

std::once_flag logging_init_flag;
bool initialized_logging = false;
std::atomic<LavLoggingCallback> saved_logging_callback;

void implicitInitLogging() {
	std::call_once(logging_init_flag, [] () {
		logger_singleton::initialize();
		initialized_logging = true;
		saved_logging_callback.store(nullptr);
	});
}

void shutdownLogging() {
	if(initialized_logging) logger_singleton::shutdown();
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
	cb(loggingLevelToInt(msg.level), outgoing.c_str(), msg.is_final);
}

//begin public api.
Lav_PUBLIC_FUNCTION LavError Lav_setLoggingCallback(LavLoggingCallback cb) {
	PUB_BEGIN
	implicitInitLogging();
	logger_singleton::getLogger()->setLoggingCallback([=] (logger_singleton::LogMessage &msg) {logCallbackTranslator(msg, cb);});
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
	logger_singleton::getLogger()->setLoggingLevel(intToLoggingLevel(level));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_getLoggingLevel(int* destination) {
	PUB_BEGIN
	implicitInitLogging();
	*destination = loggingLevelToInt(logger_singleton::getLogger()->getLoggingLevel());
	PUB_END
}

}