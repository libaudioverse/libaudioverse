#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/logging.hpp>
#include <memory>
#include <utility>

namespace audio_io {
//the functionality here is all public.

const std::pair<const char*, const implementation::OutputDeviceFactoryCreationFunction> outputDeviceFactoryCreators[] = {
	#ifdef AUDIO_IO_WINDOWS_BACKENDS
	{"Wasapi", implementation::createWasapiOutputDeviceFactory},
	{"winmm", implementation::createWinmmOutputDeviceFactory},
	#endif
};

std::shared_ptr<OutputDeviceFactory> getOutputDeviceFactory() {
	OutputDeviceFactory* fact=nullptr;
	for(int i=0; i < sizeof(outputDeviceFactoryCreators)/sizeof(outputDeviceFactoryCreators[0]); i++) {
		implementation::logInfo("Attempting to use device factory %s.", outputDeviceFactoryCreators[i].first);
		fact = outputDeviceFactoryCreators[i].second();
		if(fact) break;
	}
	if(fact == nullptr) {
		implementation::logCritical("Failed to create a device factory.  Audio output is unavailable on this system.");
		throw NoBackendError();
	}
	return std::shared_ptr<OutputDeviceFactory>(fact);
}

}