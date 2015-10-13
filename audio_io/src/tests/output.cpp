#include <audio_io/audio_io.hpp>
#include <math.h>
#include <thread>
#include <chrono>
#include <exception>

#define PI 3.14159
int channels = 2;
int sr = 44100;
int block_size = 1024;
float min_latency, start_latency, max_latency;


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
		for(int j = 0; j < channels; j++) block[i*channels+j] = sinf(2*PI*phase);
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
		if(argc != 7) {
			printf("Usage: output <channels> <sr> <block_size> <minLatency> <startLatency> <maxLatency>\n");
			return 0;
		}
		sscanf(args[1], "%i", &channels);
		sscanf(args[2], "%i", &sr);
		sscanf(args[3], "%i", &block_size);
		sscanf(args[4], "%f", &min_latency);
		sscanf(args[5], "%f", &start_latency);
		sscanf(args[6], "%f", &max_latency);
		printf("Playing with channels=%i, sr=%i, block_size=%i, min_latency=%f, start_latency=%f, max_latency=%f\n",
		channels, sr, block_size, min_latency, start_latency, max_latency);
		auto gen= SineGen(300.0);
		auto factory = audio_io::getOutputDeviceFactory();
		printf("Using factory: %s\n", factory->getName().c_str());
		auto dev = factory->createDevice(gen, -1, channels, sr, block_size, min_latency, start_latency, max_latency);
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		//We need to make sure the factory and the device die first.
		dev.reset();
		factory.reset();
		audio_io::shutdown();
	}
	catch(std::exception &e) {
		printf("Exception: %s", e.what());
	}
}
