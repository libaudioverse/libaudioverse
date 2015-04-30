/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/audio_devices.hpp>
#include <stdlib.h>
#include <functional>
#include <algorithm>
#include <iterator>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <stdarg.h>
#include <libaudioverse/private/logging.hpp>

namespace libaudioverse_implementation {

Logger *logger = nullptr;

Logger::Logger() {
	logging_thread = std::thread([this]() {loggingThreadFunction();});
	workspace = new char[workspace_length];
}

Logger::~Logger() {
	log_queue.enqueue(LogMessage(Lav_LOG_LEVEL_CRITICAL, "Logger shutting down", true));
	logging_thread.join();
}

void Logger::log(int level, std::string fmt, va_list& argptr) {
	int got = vsnprintf(workspace, workspace_length, fmt.c_str(), argptr);
	if(got == 0) return;
	LogMessage msg(level, std::string(workspace), false);
	log_queue.enqueue(msg);
}

void Logger::setLoggingLevel(int level) {
	config_mutex.lock();
	this->level = level;
	config_mutex.unlock();
}

int Logger::getLoggingLevel() {
	config_mutex.lock();
	int retval = level;
	config_mutex.unlock();
	return retval;
}

void Logger::setLoggingCallback(LavLoggingCallback cb) {
	config_mutex.lock();
	callback = cb;
	config_mutex.unlock();
}

LavLoggingCallback Logger::getLoggingCallback() {
	config_mutex.lock();
	LavLoggingCallback retval = callback;
	config_mutex.unlock();
	return retval;
}

void Logger::loggingThreadFunction() {
	bool shouldContinue = true;
	while(shouldContinue) {
		LogMessage msg= log_queue.dequeue();
		config_mutex.lock();
		if(callback && msg.level <= level) callback(msg.level, msg.message.c_str(), msg.is_final);
		config_mutex.unlock();
		if(msg.is_final) shouldContinue = false;
	}
}

std::once_flag logging_init_flag;
void initLogging() {
	logger = new Logger();
}

void log(int level, std::string fmt, ...) {
	std::call_once(logging_init_flag, initLogging);
	va_list argptr;
	va_start(argptr, fmt);
	logger->log(level, fmt, argptr);
	va_end(argptr);
}

void shutdownLogging() {
	delete logger;
}

//begin public api.
Lav_PUBLIC_FUNCTION LavError Lav_setLoggingCallback(LavLoggingCallback cb) {
	PUB_BEGIN
	std::call_once(logging_init_flag, initLogging);
	logger->setLoggingCallback(cb);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_getLoggingCallback(LavLoggingCallback* destination) {
	PUB_BEGIN
		std::call_once(logging_init_flag, initLogging);
	*destination = logger->getLoggingCallback();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_setLoggingLevel(int level) {
	PUB_BEGIN
	std::call_once(logging_init_flag, initLogging);
	logger->setLoggingLevel(level);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_getLoggingLevel(int* destination) {
	PUB_BEGIN
	std::call_once(logging_init_flag, initLogging);
	*destination = logger->getLoggingLevel();
	PUB_END
}

}