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
	last_predicted_latency = startLatency;
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
	doPrediction();
}

void LatencyPredictor::hadUnderrun() {
	double proposed = latency_increment_for_underrun+last_predicted_latency;
	if(proposed > max_allowed_latency) proposed = max_allowed_latency;
	min_allowed_latency = proposed;
	doPrediction();
}

double LatencyPredictor::predictLatency() {
	return last_predicted_latency;
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

void LatencyPredictor::doPrediction() {
	/*Rationale:
	The maximum is close to the average callback time if there is not a spike.
	The maximum is not close to the average time if the callback likes to spike.
	If the callback has not spiked for some time, then we come down because it has metaphorically proved itself.*/
	double predicted = *std::max_element(history, history+history_length);
	if(predicted < min_allowed_latency) predicted = min_allowed_latency;
	if(predicted > max_allowed_latency) predicted = max_allowed_latency;
	last_predicted_latency = predicted;
}

}
}