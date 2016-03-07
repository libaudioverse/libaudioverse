#include <audio_io/private/latency_predictor.hpp>
#include <chrono>
#include <algorithm>
#include <math.h>

namespace audio_io {
namespace implementation {

LatencyPredictor::LatencyPredictor(int historyLength, double minAllowedLatency, double startLatency, double maxAllowedLatency):
history_length(historyLength), min_allowed_latency(minAllowedLatency), max_allowed_latency(maxAllowedLatency) {
	history = new double[historyLength];
	std::fill(history, history+historyLength, startLatency);
}

LatencyPredictor::~LatencyPredictor() {
	delete[] history;
}

void LatencyPredictor::beginPass() {
	pass_start_time = std::chrono::high_resolution_clock::now();
}

void LatencyPredictor::endPass() {
	auto delta = std::chrono::high_resolution_clock::now()-pass_start_time;
	double timeInSeconds = delta.count()*decltype(delta)::period::num/(double)decltype(delta)::period::den;
	history[write_pointer] = timeInSeconds;
	write_pointer = (write_pointer + 1)%history_length;
}

double LatencyPredictor::predictLatency() {
	//Simple average.
	double sum = 0;
	for(int i = 0; i < history_length; i++) sum += history[i];
	double predicted = sum/history_length;
	if(predicted < min_allowed_latency) predicted = min_allowed_latency;
	if(predicted > max_allowed_latency) predicted = max_allowed_latency;
	return predicted;
}

int LatencyPredictor::predictLatencyInBlocks(int blockFrames, int sr) {
	double blockDuration = blockFrames/(double)sr;
	double blocksEstimate = predictLatency()/blockDuration;
	int blocks = (int)round(blocksEstimate);
	//We might fall outside the bounds. Check it.
	if(blocks*blockDuration > max_allowed_latency) blocks -= 1;
	else if(blocks*blockDuration < min_allowed_latency) blocks+=1;
	//Every backend that deals exclusively in blocks must have two.  These backends perform double buffering.
	//This consideration is more important than min and max latency, etc.
	if(blocks < 2) blocks = 2;
	return blocks;
}

}
}