#include <logger_singleton.hpp>
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
	message_queue.push(LogMessage(LoggingLevel::INFO, "logger_singleton", "Logger shutting down", true));
	l.unlock();
	enqueued_message.notify_one();
	logging_thread.join();
}

void Logger::submitMessage(LoggingLevel level, std::string topic, std::string message) {
	LogMessage msg(level, topic, message);
	std::unique_lock<std::mutex> l(mutex);
	message_queue.push(msg);
	l.unlock();
	enqueued_message.notify_one();
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

void Logger::loggingThreadFunction() {
	bool shouldContinue = true;
	while(shouldContinue) {
		std::unique_lock<std::mutex> l(mutex);
		if(message_queue.empty()) { //sleep till we get a message.
			enqueued_message.wait(l, [&]() {return message_queue.empty() == false;});
		}
		auto msg = message_queue.front();
		message_queue.pop();
		if(msg.is_final) shouldContinue = false;
		bool needsCallback = callback && msg.level >= level;
		auto cb = callback;
		//Execute callback outside the lock.  This prevents incoming messages from blocking.
		l.unlock();
		if(needsCallback) cb(msg);
	}
}

std::shared_ptr<Logger> *singleton = nullptr;
int initcount = 0;

void initialize() {
	if(initcount == 0) {
		//This works because we're friends with Logger.
		singleton = new std::shared_ptr<Logger>(new Logger());
	}
	initcount++;
}

void shutdown() {
	if(initcount == 1) {
		delete singleton;
	}
	initcount --;
}

std::shared_ptr<Logger> getLogger() {
	return *singleton;
}

}