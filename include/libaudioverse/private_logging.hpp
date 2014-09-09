/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include <utility>
#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <lambdatask/threadsafe_queue.hpp>
#include <stdarg.h>
#include <stdio.h>

class LavLogMessage {
	public:
	LavLogMessage() = default; //makes lambdatask happy, but in regards to functionality we don't use.
	LavLogMessage(int l, std::string msg, bool isFinal = false): level(l), message(msg), is_final(isFinal) {}
	int level = 0;
	std::string message;
	bool is_final = false;
};

class LavLogger {
	public:
	LavLogger();
	~LavLogger();
	void log(int level, std::string fmt, va_list &argptr);
	void setLoggingLevel(int level);
	int getLoggingLevel();
	void setLoggingCallback(LavLoggingCallback cb);
	LavLoggingCallback getLoggingCallback();
	private:
	void loggingThreadFunction();
	lambdatask::ThreadsafeQueue<LavLogMessage> log_queue;
	std::thread logging_thread;
	std::mutex config_mutex;
	LavLoggingCallback callback = nullptr;
	int level = Lav_LOG_LEVEL_INFO;
	char* workspace;
	size_t workspace_length = 8192;
};

//give everything access to the global logger.
void log(int level, std::string fmt, ...);

//deinitialization function.
void shutdownLogging();
