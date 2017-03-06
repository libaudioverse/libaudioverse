#pragma once
#include <logger_singleton/logger_singleton.hpp>
#include <memory>

namespace audio_io {
namespace implementation {

//Initialization is implicit via once_flag.
void shutdownLogger();

//The following need this extern declaration, it's not intended that code will use it directly.
extern std::shared_ptr<logger_singleton::Logger> *logger;

template<typename... ArgsT>
void logCritical(std::string message, ArgsT... formatArgs) {
	(*logger)->logCritical("audio_io", message, formatArgs...);
}

template<typename... ArgsT>
void logInfo(std::string message, ArgsT... formatArgs) {
	(*logger)->logInfo("audio_io", message, formatArgs...);
}

template<typename... ArgsT>
void logDebug(std::string message, ArgsT... formatArgs) {
	(*logger)->logDebug("audio_io", message, formatArgs...);
}

}
}