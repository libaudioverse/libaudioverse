#include <logger_singleton/logger_singleton.hpp>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <system_error>

namespace logger_singleton {

Logger::Logger() {
	format_workspace = new char[format_workspace_size];
	//This is a variation of powercores safeStartThread, extracted to avoid a dependency.
	//We have to deal with EAGAIN.
	bool retry = false;
	do {
		try {
			logging_thread = std::thread([this]() {loggingThreadFunction();});
			retry = false;
		}
		catch(std::system_error &e) {
			//This next line is (hopefully) a workaround for a critical bug in VC++2013.
			//See: https://connect.microsoft.com/VisualStudio/feedback/details/1053790
			if(e.code().value() == std::make_error_code(std::errc::resource_unavailable_try_again).value()) retry = true;
			else throw;
		}
	} while(retry);
}

Logger::~Logger() {
	std::unique_lock<std::mutex> l(mutex);
	running = false;
	l.unlock();
	check_cond.notify_one();
	logging_thread.join();
}

void Logger::submitMessage(LoggingLevel level, std::string topic, std::string message) {
	LogMessage msg(level, topic, message);
	std::unique_lock<std::mutex> l(mutex);
	message_queue.push(msg);
	l.unlock();
	check_cond.notify_one();
}

void Logger::setLoggingLevel(LoggingLevel level) {
	mutex.lock();
	this->level = level;
	mutex.unlock();
}

LoggingLevel Logger::getLoggingLevel() {
	mutex.lock();
	LoggingLevel retval = level;
	mutex.unlock();
	return retval;
}

void Logger::setLoggingCallback(std::function<void(LogMessage&)> cb) {
	mutex.lock();
	callback = cb;
	mutex.unlock();
}

void Logger::setAsForwarder(std::shared_ptr<Logger> to) {
	setLoggingCallback([=](LogMessage& m) {
		to->submitMessage(m.level, m.topic, m.message);
	});
}

void Logger::loggingThreadFunction() {
	while(true) { //Infinite because we need to check running inside the mutex.
		std::unique_lock<std::mutex> l(mutex);
		//We need to be absolutely sure to log everything, so don't allow us to exit if there's something on the queue still.
		if(running == false && message_queue.empty()) break;
		if(message_queue.empty()) { //sleep till we get a message or running changes.
			check_cond.wait(l);
			//We need to duplicate the check here, as we don't know why we woke, and it might be spureous.
			//If it's because running changed, then we get it on the next iteration.
			if(message_queue.empty()) continue;
		}
		auto msg = message_queue.front();
		message_queue.pop();
		bool needsCallback = callback && msg.level >= level;
		auto cb = callback;
		//Execute callback outside the lock.  This prevents incoming messages from blocking.
		l.unlock();
		if(needsCallback) cb(msg);
	}
}

std::shared_ptr<Logger> createLogger() {
	return std::shared_ptr<Logger>(new Logger());
}

}