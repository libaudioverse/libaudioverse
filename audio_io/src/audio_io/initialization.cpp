#include <audio_io/audio_io.hpp>
#include <logger_singleton.hpp>

namespace audio_io {

void initialize() {
	logger_singleton::initialize();
}

void shutdown() {
	logger_singleton::shutdown();
}

}