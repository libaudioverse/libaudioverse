#pragma once
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <stdio.h>

namespace  logger_singleton {

/**Logging levels.
Everything at or below the currennt level will be logged.
Default is info.
*/
enum class LoggingLevel {
	DEBUG = 10,
	INFO = 20,
	CRITICAL = 30,
	OFF = 40,
};

/**A message from the logger.

This is the class that is passed to your callback.*/
class LogMessage {
	public:
	//The level of the message.
	LoggingLevel level;
	//The message.
	std::string message;
	/*The topic.
	It is suggested that this be the name of the library which created the message, though any value is acceptible.
	Logger_singleton gives no special meaning to this field.
	*/
	std::string topic;
	private:
	LogMessage(LoggingLevel l, std::string t, std::string msg): level(l), topic(t), message(msg) {}
	friend class Logger;
};

/**The logging singleton.
Get a reference via createLogger.

You generally create one per library, and wrap it in your own API.*/
class Logger {
	public:
	~Logger();
	/**Submit a message.  There are convenience functions below which allow use of printf-style format strings.*/
	void submitMessage(LoggingLevel level, std::string topic, std::string message);
	template<typename...  ArgsT>
	void submitFormattedMessage(LoggingLevel level, std::string topic, std::string format, ArgsT... args) {
		std::unique_lock<std::mutex> l(mutex);
		//We don't get snprintf until VC++2015, so work around it.
		#if defined(_MSC_VER) && _MSC_VER < 1900
		_snprintf_s(format_workspace, format_workspace_size, _TRUNCATE, format.c_str(), args...);
		#else
		snprintf(format_workspace, format_workspace_size, format.c_str(), args...);
		#endif
		std::string msg(format_workspace);
		l.unlock();
		submitMessage(level, topic, std::string(format_workspace));
	}

	template<typename... ArgsT>
	void logCritical(std::string topic, std::string format, ArgsT... args) {
		submitFormattedMessage(LoggingLevel::CRITICAL, topic, format, args...);
	}
	
	template<typename... ArgsT>
	void logInfo(std::string topic, std::string format, ArgsT... args) {
		submitFormattedMessage(LoggingLevel::INFO, topic, format, args...);
	}
	
	template<typename... ArgsT>
	void logDebug(std::string topic, std::string format, ArgsT... args) {
		submitFormattedMessage(LoggingLevel::DEBUG, topic, format, args...);
	}
	
	/**Only log messages at or below the current logging level will be forwarded to the logging callback.*/
	void setLoggingLevel(LoggingLevel level);
	LoggingLevel getLoggingLevel();
	void setLoggingCallback(std::function<void(LogMessage&)> cb);
	//Configure the callback to forward to another logger.
	//This will keep the other logger alive until this logger dies.
	void setAsForwarder(std::shared_ptr<Logger> to);
	private:
	Logger();
	void loggingThreadFunction();
	std::thread logging_thread;
	std::mutex mutex;
	std::queue<LogMessage> message_queue;
	bool running = true;
	//Fired when either a message is enqueued or the logger is destroyed.
	std::condition_variable check_cond;
	std::function<void(LogMessage&)> callback = nullptr;
	LoggingLevel level = LoggingLevel::INFO;
	char* format_workspace;
	int format_workspace_size = 10000;
	friend std::shared_ptr<Logger> createLogger();
};

/**Make a logger singleton.*/
std::shared_ptr<Logger> createLogger();

}