#include <audio_io/audio_io.hpp>
#include <logger_singleton/logger_singleton.hpp>
#include <audio_io/private/logging.hpp>
#include <memory>
#include <thread>

namespace audio_io {
namespace implementation {

std::shared_ptr<logger_singleton::Logger> *logger = nullptr;
std::once_flag init_logger_flag;

void initializeLogger() {
	logger = new std::shared_ptr<logger_singleton::Logger>();
	*logger = logger_singleton::createLogger();
}

void shutdownLogger() {
	if(logger) delete logger;
}

std::shared_ptr<logger_singleton::Logger> getLoggerImpl() {
	std::call_once(init_logger_flag, initializeLogger);
	return *logger;
}

}

std::shared_ptr<logger_singleton::Logger> getLogger() {
	return implementation::getLoggerImpl();
}

}