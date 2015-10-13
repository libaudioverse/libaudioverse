#include <audio_io/audio_io.hpp>
#include <audio_io/private/logging.hpp>

namespace audio_io {

void initialize() {
	//Logging uses once_flag.
}

void shutdown() {
	implementation::shutdownLogger();
}

}