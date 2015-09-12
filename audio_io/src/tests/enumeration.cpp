#include <audio_io/audio_io.hpp>
#include <logger_singleton.hpp>
#include <stdio.h>
#include <exception>

int main(int argc, char** args) {
	try { //Capture any errors.
		//Initialize logging first, and install a callback that prints.
		logger_singleton::initialize();
		logger_singleton::getLogger()->setLoggingCallback([] (logger_singleton::LogMessage &msg) {
			printf("Log: %s: %s\n", msg.topic.c_str(), msg.message.c_str());
		});
		logger_singleton::getLogger()->setLoggingLevel(logger_singleton::LoggingLevel::DEBUG);
		audio_io::initialize();
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
		logger_singleton::shutdown();
	}
	catch(std::exception &e) {
		printf("Exception: %s", e.what());
	}
}
