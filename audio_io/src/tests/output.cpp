#include <audio_io/audio_io.hpp>
#include <math.h>
#include <thread>
#include <chrono>
#include <exception>

#define PI 3.14159
int channels = 2;
int sr = 44100;
int block_size = 1024;
int mixahead;


class SineGen {
	public:
	SineGen(float freq);
	void operator()(float* block, int channels);
	float phase, delta, frequency;
};

SineGen::SineGen(float freq): frequency(freq), phase(0) {
	delta= freq/sr;
}

void SineGen::operator()(float* block, int channels) {
	for(int i = 0; i < block_size; i++) {
		float value = 0.3*sinf(2*PI*phase);
		for(int j = 0; j < channels; j++) block[i*channels+j] = value;
		phase+=delta;
	}
	phase=phase-floorf(phase);
}

int main(int argc, char** args) {
	try { //capture any errors.
		audio_io::initialize();
		audio_io::getLogger()->setLoggingCallback([] (logger_singleton::LogMessage &msg) {
			printf("Log: %s: %s\n", msg.topic.c_str(), msg.message.c_str());
		});
		audio_io::getLogger()->setLoggingLevel(logger_singleton::LoggingLevel::DEBUG);
		if(argc != 5) {
			printf("Usage: output <channels> <sr> <block_size> <mixahead>\n");
			return 0;
		}
		sscanf(args[1], "%i", &channels);
		sscanf(args[2], "%i", &sr);
		sscanf(args[3], "%i", &block_size);
		sscanf(args[4], "%i", &mixahead);
		printf("Playing with channels=%i, sr=%i, block_size=%i, mixahead = %i\n",
		channels, sr, block_size, mixahead);
		auto gen= SineGen(300.0);
		auto factory = audio_io::getOutputDeviceFactory();
		printf("Using factory: %s\n", factory->getName().c_str());
		auto dev = factory->createDevice(gen, -1, channels, sr, block_size, mixahead);
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		//We need to make sure the factory and the device die first.
		dev.reset();
		factory.reset();
		audio_io::shutdown();
	}
	catch(std::exception &e) {
		printf("Exception: %s\n", e.what());
	}
}
