#include <audio_io/audio_io.hpp>
#include <stdio.h>
#include <exception>

int main(int argc, char** args) {
	try { //Capture any errors.
		audio_io::initialize();
		audio_io::getLogger()->setLoggingCallback([] (logger_singleton::LogMessage &msg) {
			printf("Log: %s: %s\n", msg.topic.c_str(), msg.message.c_str());
		});
		audio_io::getLogger()->setLoggingLevel(logger_singleton::LoggingLevel::DEBUG);
		auto factory = audio_io::getOutputDeviceFactory();
		printf("Enumerating with %s factory.\n", factory->getName().c_str());
		printf("NOTE: this test does not yet properly handle unicode.  Names may not print properly for non-Ascii devices.\n");
		printf("%i output devices detected:\n\n", (int)factory->getOutputCount());
		auto names = factory->getOutputNames();
		auto channels = factory->getOutputMaxChannels();
		auto i = names.begin();
		auto j = channels.begin();
		for(; i != names.end(); i++, j++) {
			printf("%s (channels = %i)\n", i->c_str(), (int)(*j));
		}
		factory.reset();
		audio_io::shutdown();
	}
	catch(std::exception &e) {
		printf("Exception: %s", e.what());
	}
}
